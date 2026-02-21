#include "dap_server.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "common/error_reporter.hpp"

namespace izi {
namespace dap {

// ---------------------------------------------------------------------------
// DebugSession
// ---------------------------------------------------------------------------

void DebugSession::onStatement(int line, const std::string& file) {
    DebugState current = state.load();

    bool shouldPause = false;
    std::string stopReason = "breakpoint";

    if (current == DebugState::RUNNING) {
        // Check breakpoints
        auto it = breakpoints.find(file);
        if (it != breakpoints.end() && it->second.count(line)) {
            shouldPause = true;
            stopReason = "breakpoint";
        }
    } else if (current == DebugState::STEPPING_IN) {
        // Step in: pause on the very next statement
        shouldPause = true;
        stopReason = "step";
    } else if (current == DebugState::STEPPING_OVER) {
        // Step over: pause when we are at the same or shallower call depth
        size_t depth = interp ? interp->callDepth : 0;
        if (depth <= stepDepth) {
            shouldPause = true;
            stopReason = "step";
        }
    } else if (current == DebugState::STEPPING_OUT) {
        // Step out: pause when we return to a shallower call depth
        size_t depth = interp ? interp->callDepth : 0;
        if (depth < stepDepth) {
            shouldPause = true;
            stopReason = "step";
        }
    }

    if (shouldPause) {
        pausedLine = line;
        pausedFile = file;
        state.store(DebugState::PAUSED);
        if (sendStoppedEvent) {
            sendStoppedEvent(stopReason, 1);
        }
        // Wait until resumed
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return state.load() != DebugState::PAUSED; });
    }
}

void DebugSession::onFunctionEnter(const std::string& name, int line, const std::string& file) {
    StackFrame frame;
    frame.id = nextFrameId++;
    frame.name = name;
    frame.line = line;
    frame.file = file;
    callStack.push_back(frame);
}

void DebugSession::onFunctionExit() {
    if (!callStack.empty()) {
        callStack.pop_back();
    }
}

void DebugSession::resume(DebugState newState) {
    std::unique_lock<std::mutex> lock(mtx);
    state.store(newState);
    cv.notify_all();
}

// ---------------------------------------------------------------------------
// DAPServer
// ---------------------------------------------------------------------------

DAPServer::DAPServer() = default;

DAPServer::~DAPServer() {
    if (execThread_.joinable()) {
        session_.resume(DebugState::TERMINATED);
        execThread_.join();
    }
}

void DAPServer::run() {
    // Wire up the stopped-event callback
    session_.sendStoppedEvent = [this](const std::string& reason, int threadId) {
        json body;
        body["reason"] = reason;
        body["threadId"] = threadId;
        body["allThreadsStopped"] = true;
        sendEvent("stopped", body);
    };

    while (!shouldExit_) {
        std::string msg = readMessage();
        if (msg.empty()) break;
        handleMessage(msg);
    }
}

std::string DAPServer::readMessage() {
    int contentLength = 0;
    std::string line;

    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) break;
        if (line.rfind("Content-Length: ", 0) == 0) {
            contentLength = std::stoi(line.substr(16));
        }
    }

    if (contentLength == 0) return "";

    std::string content(contentLength, '\0');
    std::cin.read(&content[0], contentLength);
    return content;
}

void DAPServer::sendMessage(const json& message) {
    std::string content = message.dump();
    std::cout << "Content-Length: " << content.size() << "\r\n\r\n" << content << std::flush;
}

void DAPServer::sendEvent(const std::string& event, const json& body) {
    json msg;
    msg["seq"] = nextSeq_++;
    msg["type"] = "event";
    msg["event"] = event;
    msg["body"] = body;
    sendMessage(msg);
}

void DAPServer::sendResponse(int requestSeq, const std::string& command, bool success,
                              const json& body, const std::string& message) {
    json msg;
    msg["seq"] = nextSeq_++;
    msg["type"] = "response";
    msg["request_seq"] = requestSeq;
    msg["command"] = command;
    msg["success"] = success;
    if (!message.empty()) msg["message"] = message;
    msg["body"] = body;
    sendMessage(msg);
}

void DAPServer::handleMessage(const std::string& content) {
    try {
        json msg = json::parse(content);
        std::string type = msg.value("type", "");
        std::string command = msg.value("command", "");
        int seq = msg.value("seq", 0);
        json args = msg.value("arguments", json::object());

        if (type != "request") return;

        if (command == "initialize") {
            auto caps = handleInitialize(seq, args);
            sendResponse(seq, command, true, caps);
            sendEvent("initialized");
        } else if (command == "launch") {
            handleLaunch(seq, args);
        } else if (command == "attach") {
            handleAttach(seq, args);
        } else if (command == "setBreakpoints") {
            handleSetBreakpoints(seq, args);
        } else if (command == "configurationDone") {
            handleConfigurationDone(seq, args);
        } else if (command == "continue") {
            handleContinue(seq, args);
        } else if (command == "next") {
            handleNext(seq, args);
        } else if (command == "stepIn") {
            handleStepIn(seq, args);
        } else if (command == "stepOut") {
            handleStepOut(seq, args);
        } else if (command == "pause") {
            handlePause(seq, args);
        } else if (command == "stackTrace") {
            handleStackTrace(seq, args);
        } else if (command == "scopes") {
            handleScopes(seq, args);
        } else if (command == "variables") {
            handleVariables(seq, args);
        } else if (command == "threads") {
            handleThreads(seq, args);
        } else if (command == "disconnect") {
            handleDisconnect(seq, args);
        } else {
            // Unknown command: respond with error
            sendResponse(seq, command, false, json::object(), "Unknown command: " + command);
        }
    } catch (const json::exception& e) {
        std::cerr << "[DAP] JSON error: " << e.what() << "\n";
    }
}

json DAPServer::handleInitialize(int /*seq*/, const json& /*args*/) {
    initialized_ = true;
    return {
        {"supportsConfigurationDoneRequest", true},
        {"supportsFunctionBreakpoints", false},
        {"supportsConditionalBreakpoints", false},
        {"supportsStepBack", false},
        {"supportsSetVariable", false},
        {"supportsRestartFrame", false},
        {"supportsGotoTargetsRequest", false},
        {"supportsStepInTargetsRequest", false},
        {"supportsCompletionsRequest", false},
        {"supportsModulesRequest", false},
        {"supportsExceptionInfoRequest", false},
        {"supportTerminateDebuggee", true}
    };
}

void DAPServer::handleLaunch(int seq, const json& args) {
    std::string program = args.value("program", "");
    bool stopOnEntry = args.value("stopOnEntry", false);

    if (program.empty()) {
        sendResponse(seq, "launch", false, json::object(), "No program specified");
        return;
    }

    sendResponse(seq, "launch", true);
    launched_ = true;
    startExecution(program, stopOnEntry);
}

void DAPServer::handleAttach(int seq, const json& /*args*/) {
    // Attach mode is not supported for tree-walking interpreter
    sendResponse(seq, "attach", false, json::object(), "Attach mode is not supported");
}

void DAPServer::handleSetBreakpoints(int seq, const json& args) {
    std::string sourcePath;
    if (args.contains("source") && args["source"].contains("path")) {
        sourcePath = args["source"]["path"];
    }

    // Clear existing breakpoints for this file
    session_.breakpoints[sourcePath].clear();

    json verifiedBreakpoints = json::array();
    if (args.contains("breakpoints")) {
        for (const auto& bp : args["breakpoints"]) {
            int line = bp.value("line", 0);
            if (line > 0) {
                session_.breakpoints[sourcePath].insert(line);
                verifiedBreakpoints.push_back({
                    {"verified", true},
                    {"line", line}
                });
            }
        }
    }

    sendResponse(seq, "setBreakpoints", true, {{"breakpoints", verifiedBreakpoints}});
}

void DAPServer::handleConfigurationDone(int seq, const json& /*args*/) {
    sendResponse(seq, "configurationDone", true);
    // If we're paused on entry, resume to the first breakpoint
    if (session_.state.load() == DebugState::PAUSED) {
        session_.resume(DebugState::RUNNING);
    }
}

void DAPServer::handleContinue(int seq, const json& /*args*/) {
    sendResponse(seq, "continue", true, {{"allThreadsContinued", true}});
    session_.resume(DebugState::RUNNING);
}

void DAPServer::handleNext(int seq, const json& /*args*/) {
    sendResponse(seq, "next", true);
    session_.stepDepth = session_.interp ? session_.interp->callDepth : 0;
    session_.resume(DebugState::STEPPING_OVER);
}

void DAPServer::handleStepIn(int seq, const json& /*args*/) {
    sendResponse(seq, "stepIn", true);
    session_.resume(DebugState::STEPPING_IN);
}

void DAPServer::handleStepOut(int seq, const json& /*args*/) {
    sendResponse(seq, "stepOut", true);
    session_.stepDepth = session_.interp ? session_.interp->callDepth : 0;
    session_.resume(DebugState::STEPPING_OUT);
}

void DAPServer::handlePause(int seq, const json& /*args*/) {
    sendResponse(seq, "pause", true);
    // Signal interpreter to pause on next statement
    DebugState expected = DebugState::RUNNING;
    session_.state.compare_exchange_strong(expected, DebugState::STEPPING_IN);
}

void DAPServer::handleStackTrace(int seq, const json& /*args*/) {
    json frames = json::array();

    // Top frame: current paused location
    if (session_.pausedLine > 0) {
        frames.push_back({
            {"id", 0},
            {"name", "<top>"},
            {"source", {{"path", session_.pausedFile}}},
            {"line", session_.pausedLine},
            {"column", 0}
        });
    }

    // Add call stack frames (most recent first, skipping the implicit top frame)
    for (auto it = session_.callStack.rbegin(); it != session_.callStack.rend(); ++it) {
        frames.push_back({
            {"id", it->id},
            {"name", it->name},
            {"source", {{"path", it->file.empty() ? session_.pausedFile : it->file}}},
            {"line", it->line > 0 ? it->line : session_.pausedLine},
            {"column", 0}
        });
    }

    sendResponse(seq, "stackTrace", true, {
        {"stackFrames", frames},
        {"totalFrames", static_cast<int>(frames.size())}
    });
}

void DAPServer::handleScopes(int seq, const json& /*args*/) {
    // We expose a single "Locals" scope
    json scopes = json::array();
    scopes.push_back({
        {"name", "Locals"},
        {"variablesReference", 1},
        {"expensive", false}
    });
    scopes.push_back({
        {"name", "Globals"},
        {"variablesReference", 2},
        {"expensive", false}
    });
    sendResponse(seq, "scopes", true, {{"scopes", scopes}});
}

void DAPServer::handleVariables(int seq, const json& args) {
    int ref = args.value("variablesReference", 0);
    json vars = json::array();

    if (session_.interp) {
        const Environment* env = nullptr;
        if (ref == 1) {
            env = session_.interp->getCurrentEnv();
        } else if (ref == 2) {
            env = session_.interp->getGlobals();
        }
        if (env) {
            vars = collectVariables(env);
        }
    }

    sendResponse(seq, "variables", true, {{"variables", vars}});
}

void DAPServer::handleThreads(int seq, const json& /*args*/) {
    sendResponse(seq, "threads", true, {
        {"threads", json::array({{{"id", 1}, {"name", "main"}}})}
    });
}

void DAPServer::handleDisconnect(int seq, const json& args) {
    sendResponse(seq, "disconnect", true);
    // Terminate the execution thread if running
    session_.resume(DebugState::TERMINATED);
    if (execThread_.joinable()) {
        execThread_.join();
    }
    sendEvent("terminated");
    shouldExit_ = true;
}

void DAPServer::startExecution(const std::string& programPath, bool stopOnEntry) {
    session_.state.store(stopOnEntry ? DebugState::PAUSED : DebugState::RUNNING);

    execThread_ = std::thread([this, programPath, stopOnEntry]() {
        // Read source file
        std::ifstream file(programPath);
        if (!file.is_open()) {
            sendEvent("output", {
                {"category", "stderr"},
                {"output", "Cannot open file: " + programPath + "\n"}
            });
            sendEvent("terminated");
            return;
        }
        std::ostringstream ss;
        ss << file.rdbuf();
        std::string source = ss.str();

        if (stopOnEntry) {
            // Notify client we are stopped on entry
            session_.pausedLine = 1;
            session_.pausedFile = programPath;
            json body;
            body["reason"] = "entry";
            body["threadId"] = 1;
            body["allThreadsStopped"] = true;
            sendEvent("stopped", body);

            // Wait for configurationDone to resume
            std::unique_lock<std::mutex> lock(session_.mtx);
            session_.cv.wait(lock, [this] {
                return session_.state.load() != DebugState::PAUSED;
            });
        }

        try {
            Lexer lex(source);
            auto tokens = lex.scanTokens();
            Parser parser(std::move(tokens), source);
            auto program = parser.parse();

            Interpreter interp(source);
            interp.setCurrentFile(programPath);
            session_.interp = &interp;
            interp.setDebugHook(&session_);

            interp.interpret(program);
        } catch (const std::exception& e) {
            sendEvent("output", {
                {"category", "stderr"},
                {"output", std::string("Runtime error: ") + e.what() + "\n"}
            });
        }

        session_.state.store(DebugState::TERMINATED);
        session_.interp = nullptr;
        sendEvent("terminated");
    });
}

json DAPServer::collectVariables(const Environment* env) const {
    json vars = json::array();
    if (!env) return vars;

    for (const auto& [name, val] : env->getAll()) {
        vars.push_back({
            {"name", name},
            {"value", valueToString(val)},
            {"type", valueType(val)},
            {"variablesReference", 0}
        });
    }
    return vars;
}

std::string DAPServer::valueToString(const Value& v) {
    if (std::holds_alternative<Nil>(v)) return "nil";
    if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? "true" : "false";
    if (std::holds_alternative<double>(v)) {
        double d = std::get<double>(v);
        if (d == static_cast<long long>(d)) {
            return std::to_string(static_cast<long long>(d));
        }
        std::ostringstream oss;
        oss << d;
        return oss.str();
    }
    if (std::holds_alternative<std::string>(v)) return "\"" + std::get<std::string>(v) + "\"";
    if (std::holds_alternative<std::shared_ptr<Array>>(v)) return "[Array]";
    if (std::holds_alternative<std::shared_ptr<Map>>(v)) return "[Map]";
    if (std::holds_alternative<std::shared_ptr<Set>>(v)) return "[Set]";
    if (std::holds_alternative<std::shared_ptr<Callable>>(v)) return "[Function]";
    if (std::holds_alternative<std::shared_ptr<Instance>>(v)) return "[Instance]";
    return "<value>";
}

std::string DAPServer::valueType(const Value& v) {
    if (std::holds_alternative<Nil>(v)) return "Nil";
    if (std::holds_alternative<bool>(v)) return "Boolean";
    if (std::holds_alternative<double>(v)) return "Number";
    if (std::holds_alternative<std::string>(v)) return "String";
    if (std::holds_alternative<std::shared_ptr<Array>>(v)) return "Array";
    if (std::holds_alternative<std::shared_ptr<Map>>(v)) return "Map";
    if (std::holds_alternative<std::shared_ptr<Set>>(v)) return "Set";
    if (std::holds_alternative<std::shared_ptr<Callable>>(v)) return "Function";
    if (std::holds_alternative<std::shared_ptr<Instance>>(v)) return "Instance";
    return "Unknown";
}

}  // namespace dap
}  // namespace izi

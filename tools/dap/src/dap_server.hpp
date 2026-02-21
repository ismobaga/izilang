#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../third_party/json.hpp"
#include "interp/interpreter.hpp"

namespace izi {
namespace dap {

using json = nlohmann::json;

// Execution state of the debug session
enum class DebugState {
    NOT_STARTED,
    RUNNING,
    PAUSED,
    STEPPING_OVER,
    STEPPING_IN,
    STEPPING_OUT,
    TERMINATED
};

// A frame in the IziLang call stack (tracked by the debug hook)
struct StackFrame {
    int id;
    std::string name;
    std::string file;
    int line;
};

// The debug hook that the interpreter calls at each statement
class DebugSession : public DebugHook {
   public:
    // breakpoints_: source path -> set of line numbers
    std::unordered_map<std::string, std::unordered_set<int>> breakpoints;

    // Current execution state (shared between DAP thread and interpreter thread)
    std::atomic<DebugState> state{DebugState::NOT_STARTED};

    // The call stack as seen by the IziLang interpreter
    std::vector<StackFrame> callStack;
    int nextFrameId = 1;

    // The interpreter (set before starting the session)
    Interpreter* interp = nullptr;

    // For pause/resume synchronization
    std::mutex mtx;
    std::condition_variable cv;

    // The line/file at which we are currently paused
    int pausedLine = 0;
    std::string pausedFile;

    // For step-over/step-out: the call depth at which stepping started
    size_t stepDepth = 0;

    // DebugHook interface
    void onStatement(int line, const std::string& file) override;
    void onFunctionEnter(const std::string& name, int line, const std::string& file) override;
    void onFunctionExit() override;

    // Send a "stopped" event back to the DAP client (set by DAPServer)
    std::function<void(const std::string& reason, int threadId)> sendStoppedEvent;

    // Resume execution
    void resume(DebugState newState = DebugState::RUNNING);
};

class DAPServer {
   public:
    DAPServer();
    ~DAPServer();

    void run();

   private:
    DebugSession session_;
    std::thread execThread_;
    bool initialized_ = false;
    bool launched_ = false;
    bool shouldExit_ = false;
    int nextSeq_ = 1;

    // DAP message I/O
    std::string readMessage();
    void sendMessage(const json& message);
    void sendEvent(const std::string& event, const json& body = json::object());
    void sendResponse(int requestSeq, const std::string& command, bool success,
                      const json& body = json::object(), const std::string& message = "");

    // Message dispatch
    void handleMessage(const std::string& content);

    // DAP request handlers
    json handleInitialize(int seq, const json& args);
    void handleLaunch(int seq, const json& args);
    void handleAttach(int seq, const json& args);
    void handleSetBreakpoints(int seq, const json& args);
    void handleConfigurationDone(int seq, const json& args);
    void handleContinue(int seq, const json& args);
    void handleNext(int seq, const json& args);
    void handleStepIn(int seq, const json& args);
    void handleStepOut(int seq, const json& args);
    void handlePause(int seq, const json& args);
    void handleStackTrace(int seq, const json& args);
    void handleScopes(int seq, const json& args);
    void handleVariables(int seq, const json& args);
    void handleDisconnect(int seq, const json& args);
    void handleThreads(int seq, const json& args);

    // Start the interpreter in a background thread for the given file
    void startExecution(const std::string& programPath, bool stopOnEntry);

    // Collect variables from the interpreter's current environment
    json collectVariables(const Environment* env) const;

    // Convert a Value to a DAP-compatible string
    static std::string valueToString(const Value& v);
    static std::string valueType(const Value& v);
};

}  // namespace dap
}  // namespace izi

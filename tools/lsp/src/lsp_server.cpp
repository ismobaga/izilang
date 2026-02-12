#include "lsp_server.hpp"
#include <iostream>
#include <sstream>

namespace izi {
namespace lsp {

LSPServer::LSPServer() = default;

void LSPServer::run() {
    while (!shouldShutdown_) {
        try {
            std::string message = readMessage();
            if (message.empty()) {
                break;
            }
            handleMessage(message);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

std::string LSPServer::readMessage() {
    std::string line;
    int contentLength = 0;
    
    // Read headers
    while (std::getline(std::cin, line)) {
        // Remove \r if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (line.empty()) {
            break;
        }
        
        if (line.find("Content-Length: ") == 0) {
            contentLength = std::stoi(line.substr(16));
        }
    }
    
    if (contentLength == 0) {
        return "";
    }
    
    // Read content
    std::string content(contentLength, '\0');
    std::cin.read(&content[0], contentLength);
    
    return content;
}

void LSPServer::sendMessage(const json& message) {
    std::string content = message.dump();
    
    std::cout << "Content-Length: " << content.length() << "\r\n";
    std::cout << "\r\n";
    std::cout << content << std::flush;
}

void LSPServer::handleMessage(const std::string& content) {
    try {
        json message = json::parse(content);
        
        std::string method = message.value("method", "");
        
        json response;
        response["jsonrpc"] = "2.0";
        
        if (message.contains("id")) {
            response["id"] = message["id"];
        }
        
        // Handle requests (need response)
        if (method == "initialize") {
            response["result"] = handleInitialize(message.value("params", json::object()));
            sendMessage(response);
        } else if (method == "shutdown") {
            response["result"] = handleShutdown(message.value("params", json::object()));
            sendMessage(response);
        } else if (method == "textDocument/completion") {
            response["result"] = handleCompletion(message.value("params", json::object()));
            sendMessage(response);
        } else if (method == "textDocument/hover") {
            response["result"] = handleHover(message.value("params", json::object()));
            sendMessage(response);
        } else if (method == "textDocument/definition") {
            response["result"] = handleDefinition(message.value("params", json::object()));
            sendMessage(response);
        } else if (method == "textDocument/references") {
            response["result"] = handleReferences(message.value("params", json::object()));
            sendMessage(response);
        } else if (method == "textDocument/rename") {
            response["result"] = handleRename(message.value("params", json::object()));
            sendMessage(response);
        } else if (method == "textDocument/documentSymbol") {
            response["result"] = handleDocumentSymbol(message.value("params", json::object()));
            sendMessage(response);
        }
        // Handle notifications (no response)
        else if (method == "exit") {
            handleExit();
        } else if (method == "initialized") {
            // Client is ready, nothing to do
        } else if (method == "textDocument/didOpen") {
            handleDidOpen(message.value("params", json::object()));
        } else if (method == "textDocument/didChange") {
            handleDidChange(message.value("params", json::object()));
        } else if (method == "textDocument/didClose") {
            handleDidClose(message.value("params", json::object()));
        } else if (method == "textDocument/didSave") {
            handleDidSave(message.value("params", json::object()));
        }
        
    } catch (const json::exception& e) {
        std::cerr << "JSON error: " << e.what() << std::endl;
    }
}

json LSPServer::handleInitialize(const json& params) {
    json capabilities = {
        {"textDocumentSync", {
            {"openClose", true},
            {"change", 1},  // Full sync
            {"save", {{"includeText", false}}}
        }},
        {"completionProvider", {
            {"triggerCharacters", json::array({"."})}
        }},
        {"hoverProvider", true},
        {"definitionProvider", true},
        {"referencesProvider", true},
        {"renameProvider", true},
        {"documentSymbolProvider", true}
    };
    
    json result = {
        {"capabilities", capabilities},
        {"serverInfo", {
            {"name", "IziLang LSP"},
            {"version", "0.1.0"}
        }}
    };
    
    return result;
}

json LSPServer::handleShutdown(const json& params) {
    shouldShutdown_ = true;
    return nullptr;
}

void LSPServer::handleExit() {
    std::exit(0);
}

void LSPServer::handleDidOpen(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    std::string text = params["textDocument"]["text"];
    
    docManager_.openDocument(uri, text);
    publishDiagnostics(uri);
}

void LSPServer::handleDidChange(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    int version = params["textDocument"]["version"];
    
    // Full sync - get the full text
    std::string text = params["contentChanges"][0]["text"];
    
    docManager_.updateDocument(uri, text, version);
    publishDiagnostics(uri);
}

void LSPServer::handleDidClose(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    docManager_.closeDocument(uri);
}

void LSPServer::handleDidSave(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    publishDiagnostics(uri);
}

json LSPServer::handleCompletion(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    Position pos = jsonToPosition(params["position"]);
    
    Document* doc = docManager_.getDocument(uri);
    if (!doc) {
        return json::array();
    }
    
    json items = json::array();
    
    // Add keywords
    for (const auto& keyword : getKeywordCompletions()) {
        items.push_back({
            {"label", keyword},
            {"kind", 14},  // Keyword
            {"detail", "keyword"}
        });
    }
    
    // Add symbols from current document
    for (const auto& symbol : getSymbolCompletions(doc)) {
        int kind = 6;  // Variable
        std::string detail = "variable";
        
        auto* symbolInfo = doc->findSymbol(symbol);
        if (symbolInfo) {
            if (symbolInfo->type == "function") {
                kind = 3;  // Function
                detail = "function";
            } else if (symbolInfo->type == "class") {
                kind = 7;  // Class
                detail = "class";
            } else if (symbolInfo->type == "parameter") {
                kind = 6;  // Variable (parameters are variables)
                detail = "parameter";
            }
        }
        
        items.push_back({
            {"label", symbol},
            {"kind", kind},
            {"detail", detail}
        });
    }
    
    return items;
}

json LSPServer::handleHover(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    Position pos = jsonToPosition(params["position"]);
    
    Document* doc = docManager_.getDocument(uri);
    if (!doc) {
        return nullptr;
    }
    
    // Find the token at the position
    size_t offset = doc->positionToOffset(pos);
    
    // Search through symbols to find one at this position
    for (const auto& [name, info] : doc->getSymbols()) {
        // Check if this position is in the definition or any reference
        size_t defStart = doc->positionToOffset(info.definition.range.start);
        size_t defEnd = doc->positionToOffset(info.definition.range.end);
        
        if (offset >= defStart && offset <= defEnd) {
            std::string contents = "**" + info.type + "** `" + name + "`\n\n";
            contents += "Defined at line " + std::to_string(info.definition.range.start.line + 1);
            
            if (!info.references.empty()) {
                contents += "\n\n" + std::to_string(info.references.size()) + " reference(s)";
            }
            
            return {
                {"contents", {
                    {"kind", "markdown"},
                    {"value", contents}
                }},
                {"range", rangeToJson(info.definition.range)}
            };
        }
        
        // Check references
        for (const auto& ref : info.references) {
            size_t refStart = doc->positionToOffset(ref.range.start);
            size_t refEnd = doc->positionToOffset(ref.range.end);
            
            if (offset >= refStart && offset <= refEnd) {
                std::string contents = "**" + info.type + "** `" + name + "`\n\n";
                contents += "Defined at line " + std::to_string(info.definition.range.start.line + 1);
                
                return {
                    {"contents", {
                        {"kind", "markdown"},
                        {"value", contents}
                    }},
                    {"range", rangeToJson(ref.range)}
                };
            }
        }
    }
    
    return nullptr;
}

json LSPServer::handleDefinition(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    Position pos = jsonToPosition(params["position"]);
    
    Document* doc = docManager_.getDocument(uri);
    if (!doc) {
        return nullptr;
    }
    
    size_t offset = doc->positionToOffset(pos);
    
    // Find the symbol at this position
    for (const auto& [name, info] : doc->getSymbols()) {
        // Check references to see if we're on one
        for (const auto& ref : info.references) {
            size_t refStart = doc->positionToOffset(ref.range.start);
            size_t refEnd = doc->positionToOffset(ref.range.end);
            
            if (offset >= refStart && offset <= refEnd) {
                return locationToJson(info.definition);
            }
        }
    }
    
    return nullptr;
}

json LSPServer::handleReferences(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    Position pos = jsonToPosition(params["position"]);
    bool includeDeclaration = params.value("context", json::object()).value("includeDeclaration", true);
    
    Document* doc = docManager_.getDocument(uri);
    if (!doc) {
        return json::array();
    }
    
    size_t offset = doc->positionToOffset(pos);
    
    // Find the symbol at this position
    for (const auto& [name, info] : doc->getSymbols()) {
        // Check if we're on the definition
        size_t defStart = doc->positionToOffset(info.definition.range.start);
        size_t defEnd = doc->positionToOffset(info.definition.range.end);
        bool onDefinition = (offset >= defStart && offset <= defEnd);
        
        // Check if we're on a reference
        bool onReference = false;
        for (const auto& ref : info.references) {
            size_t refStart = doc->positionToOffset(ref.range.start);
            size_t refEnd = doc->positionToOffset(ref.range.end);
            
            if (offset >= refStart && offset <= refEnd) {
                onReference = true;
                break;
            }
        }
        
        if (onDefinition || onReference) {
            json locations = json::array();
            
            // Add definition if requested
            if (includeDeclaration) {
                locations.push_back(locationToJson(info.definition));
            }
            
            // Add all references
            for (const auto& ref : info.references) {
                locations.push_back(locationToJson(ref));
            }
            
            return locations;
        }
    }
    
    return json::array();
}

json LSPServer::handleRename(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    Position pos = jsonToPosition(params["position"]);
    std::string newName = params["newName"];
    
    Document* doc = docManager_.getDocument(uri);
    if (!doc) {
        return nullptr;
    }
    
    size_t offset = doc->positionToOffset(pos);
    
    // Find the symbol at this position
    for (const auto& [name, info] : doc->getSymbols()) {
        // Check if we're on the definition or a reference
        size_t defStart = doc->positionToOffset(info.definition.range.start);
        size_t defEnd = doc->positionToOffset(info.definition.range.end);
        bool found = (offset >= defStart && offset <= defEnd);
        
        if (!found) {
            for (const auto& ref : info.references) {
                size_t refStart = doc->positionToOffset(ref.range.start);
                size_t refEnd = doc->positionToOffset(ref.range.end);
                
                if (offset >= refStart && offset <= refEnd) {
                    found = true;
                    break;
                }
            }
        }
        
        if (found) {
            json changes = json::object();
            json edits = json::array();
            
            // Add edit for definition
            edits.push_back({
                {"range", rangeToJson(info.definition.range)},
                {"newText", newName}
            });
            
            // Add edits for all references
            for (const auto& ref : info.references) {
                edits.push_back({
                    {"range", rangeToJson(ref.range)},
                    {"newText", newName}
                });
            }
            
            changes[uri] = edits;
            
            return {{"changes", changes}};
        }
    }
    
    return nullptr;
}

json LSPServer::handleDocumentSymbol(const json& params) {
    std::string uri = params["textDocument"]["uri"];
    
    Document* doc = docManager_.getDocument(uri);
    if (!doc) {
        return json::array();
    }
    
    json symbols = json::array();
    
    for (const auto& [name, info] : doc->getSymbols()) {
        int kind = 13;  // Variable
        if (info.type == "function") {
            kind = 12;  // Function
        } else if (info.type == "class") {
            kind = 5;  // Class
        } else if (info.type == "parameter") {
            kind = 13;  // Variable
        }
        
        symbols.push_back({
            {"name", name},
            {"kind", kind},
            {"range", rangeToJson(info.definition.range)},
            {"selectionRange", rangeToJson(info.definition.range)}
        });
    }
    
    return symbols;
}

void LSPServer::publishDiagnostics(const std::string& uri) {
    auto diagnostics = docManager_.getDiagnostics(uri);
    
    json diags = json::array();
    
    for (const auto& diag : diagnostics) {
        int severity = 1;  // Error
        if (diag.severity == SemanticDiagnostic::Severity::Warning) {
            severity = 2;  // Warning
        } else if (diag.severity == SemanticDiagnostic::Severity::Info) {
            severity = 3;  // Information
        }
        
        Position start(diag.line - 1, diag.column - 1);
        // Use a more reasonable range - highlight at least one character
        // Default to highlighting the entire line if we don't have specific end info
        Position end(diag.line - 1, diag.column > 0 ? diag.column : 1);
        
        diags.push_back({
            {"range", rangeToJson(Range(start, end))},
            {"severity", severity},
            {"source", "izilang"},
            {"message", diag.message}
        });
    }
    
    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/publishDiagnostics"},
        {"params", {
            {"uri", uri},
            {"diagnostics", diags}
        }}
    };
    
    sendMessage(notification);
}

std::vector<std::string> LSPServer::getKeywordCompletions() {
    return {
        "var", "fn", "if", "else", "while", "for", "return",
        "break", "continue", "class", "extends", "this", "super",
        "import", "export", "from", "as", "try", "catch", "finally",
        "throw", "match", "true", "false", "nil", "print", "and", "or"
    };
}

std::vector<std::string> LSPServer::getSymbolCompletions(Document* doc) {
    std::vector<std::string> symbols;
    
    for (const auto& [name, info] : doc->getSymbols()) {
        symbols.push_back(name);
    }
    
    return symbols;
}

json LSPServer::positionToJson(const Position& pos) {
    return {
        {"line", pos.line},
        {"character", pos.character}
    };
}

json LSPServer::rangeToJson(const Range& range) {
    return {
        {"start", positionToJson(range.start)},
        {"end", positionToJson(range.end)}
    };
}

json LSPServer::locationToJson(const Location& loc) {
    return {
        {"uri", loc.uri},
        {"range", rangeToJson(loc.range)}
    };
}

Position LSPServer::jsonToPosition(const json& j) {
    return Position(j["line"], j["character"]);
}

Range LSPServer::jsonToRange(const json& j) {
    return Range(jsonToPosition(j["start"]), jsonToPosition(j["end"]));
}

} // namespace lsp
} // namespace izi

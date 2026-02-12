#pragma once

#include <string>
#include <functional>
#include <vector>

#include "document_manager.hpp"
#include "../third_party/json.hpp"

namespace izi {
namespace lsp {

using json = nlohmann::json;

class LSPServer {
public:
    LSPServer();
    
    void run();
    
private:
    DocumentManager docManager_;
    bool shouldShutdown_ = false;
    
    // Message handling
    void handleMessage(const std::string& content);
    std::string readMessage();
    void sendMessage(const json& message);
    
    // LSP lifecycle
    json handleInitialize(const json& params);
    json handleShutdown(const json& params);
    void handleExit();
    
    // Text synchronization
    void handleDidOpen(const json& params);
    void handleDidChange(const json& params);
    void handleDidClose(const json& params);
    void handleDidSave(const json& params);
    
    // Language features
    json handleCompletion(const json& params);
    json handleHover(const json& params);
    json handleDefinition(const json& params);
    json handleReferences(const json& params);
    json handleRename(const json& params);
    json handleDocumentSymbol(const json& params);
    
    // Diagnostics
    void publishDiagnostics(const std::string& uri);
    
    // Helper methods
    std::vector<std::string> getKeywordCompletions();
    std::vector<std::string> getSymbolCompletions(Document* doc);
    
    // JSON conversion helpers
    json positionToJson(const Position& pos);
    json rangeToJson(const Range& range);
    json locationToJson(const Location& loc);
    Position jsonToPosition(const json& j);
    Range jsonToRange(const json& j);
};

} // namespace lsp
} // namespace izi

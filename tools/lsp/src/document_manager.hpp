#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "common/semantic_analyzer.hpp"
#include "common/token.hpp"
#include "ast/stmt.hpp"

namespace izi {
namespace lsp {

struct Position {
    int line;
    int character;

    Position() : line(0), character(0) {}
    Position(int l, int c) : line(l), character(c) {}
};

struct Range {
    Position start;
    Position end;

    Range() = default;
    Range(Position s, Position e) : start(s), end(e) {}
};

struct Location {
    std::string uri;
    Range range;

    Location() = default;
    Location(std::string u, Range r) : uri(std::move(u)), range(r) {}
};

struct SymbolInfo {
    std::string name;
    std::string type;  // "variable", "function", "class", "parameter"
    Location definition;
    std::vector<Location> references;

    SymbolInfo() = default;
    SymbolInfo(std::string n, std::string t, Location def)
        : name(std::move(n)), type(std::move(t)), definition(std::move(def)) {}
};

class Document {
public:
    explicit Document(std::string uri, std::string content)
        : uri_(std::move(uri)), content_(std::move(content)), version_(0) {}

    void setContent(std::string content, int version) {
        content_ = std::move(content);
        version_ = version;
        needsReparse_ = true;
    }

    const std::string& getUri() const { return uri_; }
    const std::string& getContent() const { return content_; }
    int getVersion() const { return version_; }

    void parse();
    const std::vector<Token>& getTokens() const { return tokens_; }
    const std::vector<StmtPtr>& getAst() const { return ast_; }
    const std::vector<SemanticDiagnostic>& getDiagnostics() const { return diagnostics_; }
    
    // Symbol information
    const std::unordered_map<std::string, SymbolInfo>& getSymbols() const { return symbols_; }
    void addSymbol(const std::string& name, SymbolInfo info);
    SymbolInfo* findSymbol(const std::string& name);

    // Position helpers
    Position offsetToPosition(size_t offset) const;
    size_t positionToOffset(const Position& pos) const;
    
    bool hasErrors() const;

private:
    std::string uri_;
    std::string content_;
    int version_;
    bool needsReparse_ = true;

    std::vector<Token> tokens_;
    std::vector<StmtPtr> ast_;
    std::vector<SemanticDiagnostic> diagnostics_;
    std::unordered_map<std::string, SymbolInfo> symbols_;
    
    void buildSymbolTable();
};

class DocumentManager {
public:
    DocumentManager() = default;

    void openDocument(const std::string& uri, const std::string& content);
    void closeDocument(const std::string& uri);
    void updateDocument(const std::string& uri, const std::string& content, int version);

    Document* getDocument(const std::string& uri);
    
    // Get all diagnostics for a document
    std::vector<SemanticDiagnostic> getDiagnostics(const std::string& uri);

private:
    std::unordered_map<std::string, std::unique_ptr<Document>> documents_;
};

} // namespace lsp
} // namespace izi

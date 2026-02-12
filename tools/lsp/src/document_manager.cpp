#include "document_manager.hpp"
#include "ast/visitor.hpp"
#include "common/error.hpp"
#include <iostream>

namespace izi {
namespace lsp {

// Symbol visitor to build symbol table
class SymbolTableBuilder : public StmtVisitor, public ExprVisitor {
public:
    SymbolTableBuilder(Document* doc) : document_(doc) {}

    void analyze(const std::vector<StmtPtr>& program) {
        for (const auto& stmt : program) {
            if (stmt) {
                stmt->accept(*this);
            }
        }
    }

    // Statement visitors
    void visit(VarStmt& stmt) override {
        // VarStmt.name is std::string, we don't have position info here
        // We'll need to track this differently, for now just add the symbol without location
        SymbolInfo info(stmt.name, "variable", Location());
        document_->addSymbol(stmt.name, std::move(info));
        
        if (stmt.initializer) {
            stmt.initializer->accept(*this);
        }
    }

    void visit(FunctionStmt& stmt) override {
        // FunctionStmt.name and params are std::string
        SymbolInfo info(stmt.name, "function", Location());
        document_->addSymbol(stmt.name, std::move(info));

        // Process parameters
        for (const auto& param : stmt.params) {
            SymbolInfo paramInfo(param, "parameter", Location());
            document_->addSymbol(param, std::move(paramInfo));
        }

        // Process body
        for (const auto& bodyStmt : stmt.body) {
            if (bodyStmt) {
                bodyStmt->accept(*this);
            }
        }
    }

    void visit(ClassStmt& stmt) override {
        // ClassStmt.name is std::string
        SymbolInfo info(stmt.name, "class", Location());
        document_->addSymbol(stmt.name, std::move(info));

        // Process methods
        for (const auto& method : stmt.methods) {
            if (method) {
                method->accept(*this);
            }
        }
    }

    void visit(ExprStmt& stmt) override {
        if (stmt.expr) {
            stmt.expr->accept(*this);
        }
    }

    void visit(BlockStmt& stmt) override {
        for (const auto& s : stmt.statements) {
            if (s) {
                s->accept(*this);
            }
        }
    }

    void visit(IfStmt& stmt) override {
        if (stmt.condition) stmt.condition->accept(*this);
        if (stmt.thenBranch) stmt.thenBranch->accept(*this);
        if (stmt.elseBranch) stmt.elseBranch->accept(*this);
    }

    void visit(WhileStmt& stmt) override {
        if (stmt.condition) stmt.condition->accept(*this);
        if (stmt.body) stmt.body->accept(*this);
    }

    void visit(ReturnStmt& stmt) override {
        if (stmt.value) stmt.value->accept(*this);
    }

    void visit(ImportStmt& stmt) override {}
    void visit(ExportStmt& stmt) override {}
    void visit(BreakStmt& stmt) override {}
    void visit(ContinueStmt& stmt) override {}
    void visit(TryStmt& stmt) override {
        if (stmt.tryBlock) stmt.tryBlock->accept(*this);
        if (stmt.catchBlock) stmt.catchBlock->accept(*this);
        if (stmt.finallyBlock) stmt.finallyBlock->accept(*this);
    }
    void visit(ThrowStmt& stmt) override {
        if (stmt.value) stmt.value->accept(*this);
    }

    // Expression visitors - track variable references
    Value visit(VariableExpr& expr) override {
        if (auto* symbol = document_->findSymbol(expr.name)) {
            // VariableExpr has name as std::string, not Token
            // We need to find the token in the document's token list
            // For now, we'll skip adding references here
        }
        return Value();
    }

    Value visit(CallExpr& expr) override {
        if (expr.callee) expr.callee->accept(*this);
        for (const auto& arg : expr.args) {
            if (arg) arg->accept(*this);
        }
        return Value();
    }

    Value visit(BinaryExpr& expr) override {
        if (expr.left) expr.left->accept(*this);
        if (expr.right) expr.right->accept(*this);
        return Value();
    }

    Value visit(UnaryExpr& expr) override {
        if (expr.right) expr.right->accept(*this);
        return Value();
    }

    Value visit(GroupingExpr& expr) override {
        if (expr.expression) expr.expression->accept(*this);
        return Value();
    }

    Value visit(LiteralExpr& expr) override { return Value(); }
    
    Value visit(AssignExpr& expr) override {
        if (auto* symbol = document_->findSymbol(expr.name)) {
            // AssignExpr has name as std::string, not Token
            // Skip adding references here for now
        }
        if (expr.value) expr.value->accept(*this);
        return Value();
    }

    Value visit(ArrayExpr& expr) override {
        for (const auto& elem : expr.elements) {
            if (elem) elem->accept(*this);
        }
        return Value();
    }

    Value visit(MapExpr& expr) override {
        for (const auto& [key, value] : expr.entries) {
            if (value) value->accept(*this);
        }
        return Value();
    }

    Value visit(IndexExpr& expr) override {
        if (expr.collection) expr.collection->accept(*this);
        if (expr.index) expr.index->accept(*this);
        return Value();
    }

    Value visit(SetIndexExpr& expr) override {
        if (expr.collection) expr.collection->accept(*this);
        if (expr.index) expr.index->accept(*this);
        if (expr.value) expr.value->accept(*this);
        return Value();
    }

    Value visit(FunctionExpr& expr) override {
        for (const auto& stmt : expr.body) {
            if (stmt) stmt->accept(*this);
        }
        return Value();
    }

    Value visit(MatchExpr& expr) override {
        if (expr.value) expr.value->accept(*this);
        for (const auto& matchCase : expr.cases) {
            if (matchCase.result) matchCase.result->accept(*this);
        }
        return Value();
    }

    Value visit(PropertyExpr& expr) override {
        if (expr.object) expr.object->accept(*this);
        return Value();
    }

    Value visit(SetPropertyExpr& expr) override {
        if (expr.object) expr.object->accept(*this);
        if (expr.value) expr.value->accept(*this);
        return Value();
    }

    Value visit(ThisExpr& expr) override { return Value(); }
    Value visit(SuperExpr& expr) override { return Value(); }

private:
    Document* document_;
};

// Document implementation
void Document::parse() {
    if (!needsReparse_) {
        return;
    }

    tokens_.clear();
    ast_.clear();
    diagnostics_.clear();
    symbols_.clear();

    try {
        // Lexing
        Lexer lexer(content_);
        tokens_ = lexer.scanTokens();

        // Parsing
        Parser parser(tokens_, content_);
        ast_ = parser.parse();

        // Semantic analysis
        SemanticAnalyzer analyzer;
        analyzer.analyze(ast_);
        diagnostics_ = analyzer.getDiagnostics();

        // Build symbol table
        buildSymbolTable();

    } catch (const LexerError& e) {
        SemanticDiagnostic diag(
            SemanticDiagnostic::Severity::Error,
            e.what(),
            e.line,
            e.column
        );
        diagnostics_.push_back(diag);
    } catch (const ParserError& e) {
        SemanticDiagnostic diag(
            SemanticDiagnostic::Severity::Error,
            e.what(),
            e.token.line,
            e.token.column
        );
        diagnostics_.push_back(diag);
    } catch (const std::exception& e) {
        SemanticDiagnostic diag(
            SemanticDiagnostic::Severity::Error,
            e.what(),
            1,
            1
        );
        diagnostics_.push_back(diag);
    }

    needsReparse_ = false;
}

void Document::buildSymbolTable() {
    SymbolTableBuilder builder(this);
    builder.analyze(ast_);
}

void Document::addSymbol(const std::string& name, SymbolInfo info) {
    symbols_[name] = std::move(info);
}

SymbolInfo* Document::findSymbol(const std::string& name) {
    auto it = symbols_.find(name);
    if (it != symbols_.end()) {
        return &it->second;
    }
    return nullptr;
}

Position Document::offsetToPosition(size_t offset) const {
    int line = 0;
    int character = 0;
    
    for (size_t i = 0; i < offset && i < content_.size(); ++i) {
        if (content_[i] == '\n') {
            line++;
            character = 0;
        } else {
            character++;
        }
    }
    
    return Position(line, character);
}

size_t Document::positionToOffset(const Position& pos) const {
    int currentLine = 0;
    size_t offset = 0;
    
    for (size_t i = 0; i < content_.size(); ++i) {
        if (currentLine == pos.line && offset == static_cast<size_t>(pos.character)) {
            return i;
        }
        
        if (content_[i] == '\n') {
            currentLine++;
            offset = 0;
        } else {
            offset++;
        }
    }
    
    return content_.size();
}

bool Document::hasErrors() const {
    for (const auto& diag : diagnostics_) {
        if (diag.severity == SemanticDiagnostic::Severity::Error) {
            return true;
        }
    }
    return false;
}

// DocumentManager implementation
void DocumentManager::openDocument(const std::string& uri, const std::string& content) {
    auto doc = std::make_unique<Document>(uri, content);
    doc->parse();
    documents_[uri] = std::move(doc);
}

void DocumentManager::closeDocument(const std::string& uri) {
    documents_.erase(uri);
}

void DocumentManager::updateDocument(const std::string& uri, const std::string& content, int version) {
    auto it = documents_.find(uri);
    if (it != documents_.end()) {
        it->second->setContent(content, version);
        it->second->parse();
    }
}

Document* DocumentManager::getDocument(const std::string& uri) {
    auto it = documents_.find(uri);
    if (it != documents_.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<SemanticDiagnostic> DocumentManager::getDiagnostics(const std::string& uri) {
    auto* doc = getDocument(uri);
    if (doc) {
        return doc->getDiagnostics();
    }
    return {};
}

} // namespace lsp
} // namespace izi

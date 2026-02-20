#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "ast/stmt.hpp"
#include "ast/expr.hpp"
#include "ast/visitor.hpp"
#include "ast/type.hpp"
#include "common/token.hpp"
#include "common/value.hpp"

namespace izi {

// Semantic analysis errors and warnings
struct SemanticDiagnostic {
    enum class Severity { Error, Warning, Info };

    Severity severity;
    std::string message;
    int line;
    int column;

    SemanticDiagnostic(Severity sev, std::string msg, int ln, int col)
        : severity(sev), message(std::move(msg)), line(ln), column(col) {}
};

// Semantic analyzer performs static analysis on the AST
// Phase 1: Type checking (when type annotations present)
// Phase 2: Dead code detection, unused variables, unreachable code
class SemanticAnalyzer : public ExprVisitor, public StmtVisitor {
   public:
    SemanticAnalyzer() = default;

    // Analyze a program and collect diagnostics
    void analyze(const std::vector<StmtPtr>& program);

    // Get collected diagnostics
    const std::vector<SemanticDiagnostic>& getDiagnostics() const { return diagnostics_; }

    bool hasErrors() const {
        for (const auto& diag : diagnostics_) {
            if (diag.severity == SemanticDiagnostic::Severity::Error) {
                return true;
            }
        }
        return false;
    }

    // ExprVisitor interface (returns inferred type)
    Value visit(BinaryExpr& expr) override;
    Value visit(UnaryExpr& expr) override;
    Value visit(LiteralExpr& expr) override;
    Value visit(GroupingExpr& expr) override;
    Value visit(ConditionalExpr& expr) override;
    Value visit(CallExpr& expr) override;
    Value visit(VariableExpr& expr) override;
    Value visit(AssignExpr& expr) override;
    Value visit(ArrayExpr& expr) override;
    Value visit(MapExpr& expr) override;
    Value visit(SpreadExpr& expr) override;
    Value visit(IndexExpr& expr) override;
    Value visit(SetIndexExpr& expr) override;
    Value visit(FunctionExpr& expr) override;
    Value visit(MatchExpr& expr) override;
    Value visit(PropertyExpr& expr) override;
    Value visit(SetPropertyExpr& expr) override;
    Value visit(ThisExpr& expr) override;
    Value visit(SuperExpr& expr) override;

    // StmtVisitor interface
    void visit(ExprStmt& stmt) override;
    void visit(BlockStmt& stmt) override;
    void visit(VarStmt& stmt) override;
    void visit(WhileStmt& stmt) override;
    void visit(IfStmt& stmt) override;
    void visit(FunctionStmt& stmt) override;
    void visit(ReturnStmt& stmt) override;
    void visit(ImportStmt& stmt) override;
    void visit(ExportStmt& stmt) override;
    void visit(BreakStmt& stmt) override;
    void visit(ContinueStmt& stmt) override;
    void visit(TryStmt& stmt) override;
    void visit(ThrowStmt& stmt) override;
    void visit(ClassStmt& stmt) override;

   private:
    std::vector<SemanticDiagnostic> diagnostics_;

    // Symbol table for type tracking
    struct Scope {
        std::unordered_map<std::string, TypePtr> variables;
        std::unordered_set<std::string> usedVariables;
        std::shared_ptr<Scope> parent;
    };
    std::shared_ptr<Scope> currentScope_;

    // Control flow tracking
    bool inLoop_ = false;
    bool inFunction_ = false;
    bool hasReturnedInCurrentBlock_ = false;
    bool inMethod_ = false;  // Track if we're inside a class method
    std::string currentClassName_;  // Track current class name for validation

    // Class tracking for duplicate detection
    std::unordered_set<std::string> currentClassFields_;
    std::unordered_set<std::string> currentClassMethods_;

    // Helper methods
    void addError(const std::string& message, int line, int column);
    void addWarning(const std::string& message, int line, int column);
    void addInfo(const std::string& message, int line, int column);

    void enterScope();
    void exitScope();
    void defineVariable(const std::string& name, TypePtr type, int line, int column);
    void markVariableUsed(const std::string& name);
    TypeAnnotation* lookupVariable(const std::string& name);

    // Type inference and checking
    TypePtr inferType(Expr& expr);
    bool areTypesCompatible(const TypeAnnotation& expected, const TypeAnnotation& actual);
    TypePtr valueToType(const Value& value);

    // Helper to check if a type is explicitly annotated (not Any)
    bool isExplicitlyTyped(const TypeAnnotation* type);
};

}  // namespace izi

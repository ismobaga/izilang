#pragma once

#include <string>
#include <vector>

#include "ast/visitor.hpp"
#include "ast/expr.hpp"
#include "ast/stmt.hpp"
#include "common/value.hpp"

namespace izi {

/**
 * AstPrinter: debug/pretty-print visitor for the IziLang AST.
 *
 * Produces a compact S-expression representation of the AST tree,
 * useful for debugging, testing, and understanding parsed structure.
 *
 * Example output:
 *   var x = 1 + 2;  →  (var x (binary + 1 2))
 *   fn f(a) { return a; }  →  (fn f (a) (return a))
 */
class AstPrinter : public ExprVisitor, public StmtVisitor {
   public:
    AstPrinter() = default;

    // Print a complete program (list of statements).
    // Each statement is separated by a newline.
    std::string print(const std::vector<StmtPtr>& program);

    // Print a single statement.
    std::string printStmt(Stmt& stmt);

    // Print a single expression.
    std::string printExpr(Expr& expr);

    // ExprVisitor overrides — each stores result in result_
    Value visit(LiteralExpr& expr) override;
    Value visit(VariableExpr& expr) override;
    Value visit(AssignExpr& expr) override;
    Value visit(BinaryExpr& expr) override;
    Value visit(UnaryExpr& expr) override;
    Value visit(GroupingExpr& expr) override;
    Value visit(CallExpr& expr) override;
    Value visit(ConditionalExpr& expr) override;
    Value visit(ArrayExpr& expr) override;
    Value visit(MapExpr& expr) override;
    Value visit(SpreadExpr& expr) override;
    Value visit(IndexExpr& expr) override;
    Value visit(SetIndexExpr& expr) override;
    Value visit(PropertyExpr& expr) override;
    Value visit(SetPropertyExpr& expr) override;
    Value visit(ThisExpr& expr) override;
    Value visit(SuperExpr& expr) override;
    Value visit(FunctionExpr& expr) override;
    Value visit(AwaitExpr& expr) override;
    Value visit(MatchExpr& expr) override;

    // StmtVisitor overrides — each appends to output_
    void visit(ExprStmt& stmt) override;
    void visit(BlockStmt& stmt) override;
    void visit(VarStmt& stmt) override;
    void visit(WhileStmt& stmt) override;
    void visit(ReturnStmt& stmt) override;
    void visit(FunctionStmt& stmt) override;
    void visit(IfStmt& stmt) override;
    void visit(ImportStmt& stmt) override;
    void visit(ExportStmt& stmt) override;
    void visit(ReExportStmt& stmt) override;
    void visit(BreakStmt& stmt) override;
    void visit(ContinueStmt& stmt) override;
    void visit(TryStmt& stmt) override;
    void visit(ThrowStmt& stmt) override;
    void visit(ClassStmt& stmt) override;

   private:
    std::string result_;  // Result of the most recent expression visit
    std::string output_;  // Accumulated statement output

    // Helper: format a literal Value as a display string
    static std::string literalToString(const Value& v);

    // Helper: build an S-expression node from a tag and child strings
    static std::string sexp(const std::string& tag, const std::vector<std::string>& children);

    // Helper: format a parameter list as "(p1 p2 ...)"
    static std::string formatParams(const std::vector<std::string>& params);
};

}  // namespace izi

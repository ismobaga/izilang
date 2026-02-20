#pragma once

#include "ast/stmt.hpp"
#include "ast/expr.hpp"
#include "ast/visitor.hpp"
#include "common/value.hpp"
#include <memory>
#include <vector>

namespace izi {

/**
 * AST optimizer that performs various optimization passes:
 * - Constant folding: Evaluate constant expressions at compile time
 * - Dead code elimination: Remove unreachable code
 */
class Optimizer : public ExprVisitor, public StmtVisitor {
   public:
    Optimizer() = default;

    // Optimize a program (vector of statements)
    std::vector<StmtPtr> optimize(std::vector<StmtPtr> program);

    // Expression visitors - return optimized expression wrapped in Value
    Value visit(BinaryExpr& expr) override;
    Value visit(UnaryExpr& expr) override;
    Value visit(CallExpr& expr) override;
    Value visit(LiteralExpr& expr) override;
    Value visit(AssignExpr& expr) override;
    Value visit(VariableExpr& expr) override;
    Value visit(GroupingExpr& expr) override;
    Value visit(ConditionalExpr& expr) override;
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

    // Statement visitors
    void visit(ExprStmt& stmt) override;
    void visit(ReturnStmt& stmt) override;
    void visit(IfStmt& stmt) override;
    void visit(WhileStmt& stmt) override;
    void visit(BlockStmt& stmt) override;
    void visit(VarStmt& stmt) override;
    void visit(FunctionStmt& stmt) override;
    void visit(ImportStmt& stmt) override;
    void visit(ExportStmt& stmt) override;
    void visit(BreakStmt& stmt) override;
    void visit(ContinueStmt& stmt) override;
    void visit(TryStmt& stmt) override;
    void visit(ThrowStmt& stmt) override;
    void visit(ClassStmt& stmt) override;

   private:
    // Current optimized expression (used during expression optimization)
    ExprPtr currentExpr;

    // Current optimized statement (used during statement optimization)
    StmtPtr currentStmt;

    // Current optimized statements (used during block optimization)
    std::vector<StmtPtr> currentStmts;

    // Helper: Optimize an expression and return the optimized version
    ExprPtr optimizeExpr(ExprPtr expr);

    // Helper: Optimize a statement and return the optimized version
    StmtPtr optimizeStmt(StmtPtr stmt);

    // Helper: Check if an expression is a constant literal
    bool isConstant(const Expr& expr) const;

    // Helper: Evaluate a constant binary expression
    Value evaluateConstantBinary(const Value& left, TokenType op, const Value& right);

    // Helper: Evaluate a constant unary expression
    Value evaluateConstantUnary(TokenType op, const Value& right);
};

}  // namespace izi

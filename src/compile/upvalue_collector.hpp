// Simplified upvalue analysis for bytecode compilation
// This visitor collects variable names referenced in an expression that
// are not parameters or local declarations.

#pragma once

#include "ast/expr.hpp"
#include "ast/stmt.hpp"
#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

namespace izi {

class UpvalueCollector : public ExprVisitor, public StmtVisitor {
    public:
    // Collect upvalues from an expression and a list of local variable names
    static std::vector<std::string> collect(Expr& expr, const std::vector<std::string>& locals) {
        UpvalueCollector collector(locals);
        expr.accept(collector);
        
        // Convert set to vector and return
        std::vector<std::string> result(collector.upvalues_.begin(), collector.upvalues_.end());
        return result;
    }

    // Collect upvalues from a list of statements
    static std::vector<std::string> collectFromStatements(const std::vector<StmtPtr>& stmts, 
                                                          const std::vector<std::string>& locals) {
        UpvalueCollector collector(locals);
        for (const auto& stmt : stmts) {
            stmt->accept(collector);
        }
        
        std::vector<std::string> result(collector.upvalues_.begin(), collector.upvalues_.end());
        return result;
    }

    private:
    UpvalueCollector(const std::vector<std::string>& locals) : locals_(locals) {}

    std::unordered_set<std::string> upvalues_;
    std::vector<std::string> locals_;

    void addIfNotLocal(const std::string& name) {
        auto it = std::find(locals_.begin(), locals_.end(), name);
        if (it == locals_.end()) {
            upvalues_.insert(name);
        }
    }

    // Visitor methods
    Value visit(BinaryExpr& expr) override {
        expr.left->accept(*this);
        expr.right->accept(*this);
        return Nil{};
    }

    Value visit(UnaryExpr& expr) override {
        expr.right->accept(*this);
        return Nil{};
    }

    Value visit(CallExpr& expr) override {
        expr.callee->accept(*this);
        for (const auto& arg : expr.args) {
            arg->accept(*this);
        }
        return Nil{};
    }

    Value visit(LiteralExpr& expr) override {
        return Nil{};
    }

    Value visit(AssignExpr& expr) override {
        addIfNotLocal(expr.name);
        expr.value->accept(*this);
        return Nil{};
    }

    Value visit(VariableExpr& expr) override {
        addIfNotLocal(expr.name);
        return Nil{};
    }

    Value visit(GroupingExpr& expr) override {
        expr.expression->accept(*this);
        return Nil{};
    }

    Value visit(ConditionalExpr& expr) override {
        expr.condition->accept(*this);
        expr.thenExpr->accept(*this);
        expr.elseExpr->accept(*this);
        return Nil{};
    }

    Value visit(ArrayExpr& expr) override {
        for (const auto& elem : expr.elements) {
            elem->accept(*this);
        }
        return Nil{};
    }

    Value visit(MapExpr& expr) override {
        for (const auto& [key, val] : expr.entries) {
            val->accept(*this);
        }
        return Nil{};
    }

    Value visit(SpreadExpr& expr) override {
        expr.argument->accept(*this);
        return Nil{};
    }

    Value visit(IndexExpr& expr) override {
        expr.object->accept(*this);
        expr.index->accept(*this);
        return Nil{};
    }

    Value visit(SetIndexExpr& expr) override {
        expr.object->accept(*this);
        expr.index->accept(*this);
        expr.value->accept(*this);
        return Nil{};
    }

    Value visit(FunctionExpr& expr) override {
        // Don't descend into nested functions
        return Nil{};
    }

    Value visit(MatchExpr& expr) override {
        expr.value->accept(*this);
        // Note: pattern matching is complex, skip for now
        return Nil{};
    }

    Value visit(PropertyExpr& expr) override {
        expr.object->accept(*this);
        return Nil{};
    }

    Value visit(SetPropertyExpr& expr) override {
        expr.object->accept(*this);
        expr.value->accept(*this);
        return Nil{};
    }

    Value visit(ThisExpr& expr) override {
        return Nil{};
    }

    Value visit(SuperExpr& expr) override {
        return Nil{};
    }

    Value visit(AwaitExpr& expr) override {
        expr.argument->accept(*this);
        return Nil{};
    }

    // Statement visitors (for completeness)
    void visit(ExprStmt& stmt) override {
        stmt.expr->accept(*this);
    }

    void visit(ReturnStmt& stmt) override {
        if (stmt.value) stmt.value->accept(*this);
    }

    void visit(IfStmt& stmt) override {
        stmt.condition->accept(*this);
        stmt.thenBranch->accept(*this);
        if (stmt.elseBranch) stmt.elseBranch->accept(*this);
    }

    void visit(WhileStmt& stmt) override {
        stmt.condition->accept(*this);
        stmt.body->accept(*this);
    }

    void visit(BlockStmt& stmt) override {
        for (const auto& s : stmt.statements) {
            s->accept(*this);
        }
    }

    void visit(VarStmt& stmt) override {
        if (stmt.initializer) stmt.initializer->accept(*this);
        if (!stmt.name.empty()) locals_.push_back(stmt.name);
    }

    void visit(FunctionStmt& stmt) override {
        if (!stmt.name.empty()) locals_.push_back(stmt.name);
    }

    void visit(ImportStmt& stmt) override {}
    void visit(ExportStmt& stmt) override {}
    void visit(ReExportStmt& stmt) override {}
    void visit(BreakStmt& stmt) override {}
    void visit(ContinueStmt& stmt) override {}

    void visit(TryStmt& stmt) override {
        stmt.tryBlock->accept(*this);
        if (stmt.catchBlock) stmt.catchBlock->accept(*this);
        if (stmt.finallyBlock) stmt.finallyBlock->accept(*this);
    }

    void visit(ThrowStmt& stmt) override {
        stmt.value->accept(*this);
    }

    void visit(ClassStmt& stmt) override {
        // Don't descend into classes
    }
};

}  // namespace izi

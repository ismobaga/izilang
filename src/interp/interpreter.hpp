#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

#include "ast/stmt.hpp"
#include "ast/visitor.hpp"
#include "common/token.hpp"
#include "common/value.hpp"
#include "environment.hpp"
#include "user_function.hpp"

namespace izi {

class RuntimeError : public std::runtime_error {
   public:
    Token token;

    RuntimeError(const Token& token, const std::string& message)
        : std::runtime_error(message), token(token) {}
};

struct ReturnSignal {
    Value value;
};

class Interpreter : public ExprVisitor, public StmtVisitor {
   public:
    explicit Interpreter(std::string_view source = "");

    void interpret(const std::vector<StmtPtr>& program);
    void defineGlobal(const std::string& name, const Value& value) {
        globals.define(name, value);
    }

    // ExprVisitor
    Value visit(BinaryExpr& expr) override;
    Value visit(UnaryExpr& expr) override;
    Value visit(LiteralExpr& expr) override;
    Value visit(GroupingExpr& expr) override;
    Value visit(CallExpr& expr) override;
    Value visit(VariableExpr& expr) override;
    Value visit(AssignExpr& expr) override;
    Value visit(ArrayExpr& expr) override;
    Value visit(MapExpr& expr) override;
    Value visit(IndexExpr& expr) override;
    Value visit(SetIndexExpr& expr) override;

    // StmVisitor

    void visit(ExprStmt&) override;
    void visit(BlockStmt&) override;
    void visit(VarStmt&) override;
    void visit(WhileStmt&) override;
    void visit(IfStmt&) override;
    void visit(FunctionStmt&) override;
    void visit(ReturnStmt&) override;
    void visit(ImportStmt&) override;

    void executeBlock(const std::vector<StmtPtr>& statements, Environment* newEnv);

   private:
    std::string_view source_;
    Environment globals;
    Environment* env;

    Value evaluate(Expr& expr);
    void execute(Stmt& expr);
    
    // Helper to convert value to number with proper error
    double toNumber(const Value& v, const Token& token);
      
    
    // for imports
    std::unordered_set<std::string> importedModules;

    // helpers
    std::string loadFile(const std::string& path);
    std::string normalizeModulePath(const std::string& path);
};

}  // namespace izi
#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "ast/stmt.hpp"
#include "ast/visitor.hpp"
#include "common/token.hpp"
#include "common/value.hpp"
#include "environment.hpp"
#include "user_function.hpp"

namespace izi {

// Runtime safety limits
constexpr size_t MAX_CALL_DEPTH = 256;

class RuntimeError : public std::runtime_error {
   public:
    Token token;

    RuntimeError(const Token& token, const std::string& message)
        : std::runtime_error(message), token(token) {}
};

struct ReturnSignal {
    Value value;
};

struct BreakSignal {};

struct ContinueSignal {};

struct ThrowSignal {
    Value exception;
    Token token;  // For error reporting
    
    ThrowSignal(Value ex, Token tok)
        : exception(std::move(ex)), token(std::move(tok)) {}
};

class Interpreter : public ExprVisitor, public StmtVisitor {
   public:
    explicit Interpreter(std::string_view source = "");

    void interpret(const std::vector<StmtPtr>& program);
    void defineGlobal(const std::string& name, const Value& value) {
        globals->define(name, value);
    }

    void setCommandLineArgs(const std::vector<std::string>& args) {
        commandLineArgs = args;
    }

    const std::vector<std::string>& getCommandLineArgs() const {
        return commandLineArgs;
    }
    
    // Get global environment (for REPL :vars command)
    std::shared_ptr<Environment> getGlobals() const {
        return globals;
    }

    // ExprVisitor
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
    Value visit(PropertyExpr& expr) override;      // v0.3
    Value visit(SetPropertyExpr& expr) override;   // v0.3
    Value visit(ThisExpr& expr) override;          // v0.3
    Value visit(SuperExpr& expr) override;         // v0.3

    // StmVisitor

    void visit(ExprStmt&) override;
    void visit(BlockStmt&) override;
    void visit(VarStmt&) override;
    void visit(WhileStmt&) override;
    void visit(IfStmt&) override;
    void visit(FunctionStmt&) override;
    void visit(ReturnStmt&) override;
    void visit(ImportStmt&) override;
    void visit(ExportStmt&) override;
    void visit(BreakStmt&) override;
    void visit(ContinueStmt&) override;
    void visit(TryStmt&) override;
    void visit(ThrowStmt&) override;
    void visit(ClassStmt&) override;              // v0.3

    void executeBlock(const std::vector<StmtPtr>& statements, std::shared_ptr<Environment> newEnv);
    
    // Runtime safety tracking (public so UserFunction can access it)
    size_t callDepth = 0;

   private:
    std::string_view source_;
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> env;

    Value evaluate(Expr& expr);
    void execute(Stmt& expr);
    
    // Helper to convert value to number with proper error
    double toNumber(const Value& v, const Token& token);
      
    // for imports
    std::unordered_set<std::string> importedModules;
    std::vector<std::string> importStack;  // Track files being imported (for circular detection)
    std::string currentFile;  // Current file being executed

    // command line arguments for std.process
    std::vector<std::string> commandLineArgs;

    // helpers
    std::string loadFile(const std::string& path);
    std::string normalizeModulePath(const std::string& path);
    
public:
    // Set current file for relative import resolution
    void setCurrentFile(const std::string& filename) { currentFile = filename; }
    const std::string& getCurrentFile() const { return currentFile; }
};

}  // namespace izi
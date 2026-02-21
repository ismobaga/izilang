#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ast/stmt.hpp"
#include "ast/visitor.hpp"
#include "common/token.hpp"
#include "common/value.hpp"
#include "environment.hpp"
#include "environment_arena.hpp"
#include "user_function.hpp"

namespace izi {

// Runtime safety limits
constexpr size_t MAX_CALL_DEPTH = 256;

class RuntimeError : public std::runtime_error {
   public:
    Token token;

    RuntimeError(const Token& token, const std::string& message) : std::runtime_error(message), token(token) {}
};

struct ReturnSignal {
    Value value;
};

struct BreakSignal {};

struct ContinueSignal {};

struct ThrowSignal {
    Value exception;
    Token token;  // For error reporting

    ThrowSignal(Value ex, Token tok) : exception(std::move(ex)), token(std::move(tok)) {}
};

// Debug hook interface: implement to receive execution events from the interpreter.
// Used by the Debug Adapter Protocol (DAP) server for breakpoints and stepping.
struct DebugHook {
    virtual ~DebugHook() = default;
    // Called before each statement is executed (line is the source line number, 0 if unknown)
    virtual void onStatement(int line, const std::string& file) = 0;
    // Called when entering a named function (used to build the call stack)
    virtual void onFunctionEnter(const std::string& name, int line, const std::string& file) = 0;
    // Called when exiting a function
    virtual void onFunctionExit() = 0;
};

class Interpreter : public ExprVisitor, public StmtVisitor {
   public:
    explicit Interpreter(std::string_view source = "");

    void interpret(const std::vector<StmtPtr>& program);
    void defineGlobal(const std::string& name, const Value& value) { globals->define(name, value); }

    void setCommandLineArgs(const std::vector<std::string>& args) { commandLineArgs = args; }

    const std::vector<std::string>& getCommandLineArgs() const { return commandLineArgs; }

    // Get global environment (for REPL :vars command).
    // Returns a non-owning pointer; the environment is owned by arena_.
    const Environment* getGlobals() const { return globals; }

    // Set a debug hook to receive execution events (for DAP support).
    // The hook must outlive the interpreter. Pass nullptr to disable.
    void setDebugHook(DebugHook* hook) { debugHook_ = hook; }

    // Get the current environment (for variable inspection during debugging).
    const Environment* getCurrentEnv() const { return env; }

    // Notify the debug hook of a function entry (called by UserFunction).
    void notifyFunctionEnter(const std::string& name, int line) {
        if (debugHook_) debugHook_->onFunctionEnter(name, line, currentFile);
    }
    // Notify the debug hook of a function exit (called by UserFunction).
    void notifyFunctionExit() {
        if (debugHook_) debugHook_->onFunctionExit();
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
    Value visit(PropertyExpr& expr) override;  // v0.3
    Value visit(SetPropertyExpr& expr) override;  // v0.3
    Value visit(ThisExpr& expr) override;  // v0.3
    Value visit(SuperExpr& expr) override;  // v0.3

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
    void visit(ReExportStmt&) override;
    void visit(BreakStmt&) override;
    void visit(ContinueStmt&) override;
    void visit(TryStmt&) override;
    void visit(ThrowStmt&) override;
    void visit(ClassStmt&) override;  // v0.3

    void executeBlock(const std::vector<StmtPtr>& statements, Environment* newEnv);

    // Runtime safety tracking (public so UserFunction can access it)
    size_t callDepth = 0;

    // Arena that owns all Environment objects created during interpretation.
    // Exposed so that UserFunction::call and BoundMethod::call can allocate
    // call-frame environments without going through a separate factory.
    EnvironmentArena arena_;

   private:
    std::string_view source_;
    Environment* globals;  // Non-owning; owned by arena_
    Environment* env;      // Non-owning; owned by arena_

    // Debug hook (optional, not owned)
    DebugHook* debugHook_ = nullptr;

    Value evaluate(Expr& expr);
    void execute(Stmt& expr);

    // Helper to convert value to number with proper error
    double toNumber(const Value& v, const Token& token);

    // for imports
    std::unordered_set<std::string> importedModules;
    std::vector<std::string> importStack;  // Track files being imported (for circular detection)
    std::string currentFile;  // Current file being executed

    // Module export tracking for namespace isolation
    // Points to the exports map of the module currently being loaded (null at top level)
    std::unordered_map<std::string, Value>* currentModuleExports_ = nullptr;
    // Cache: canonical path -> exports map (populated when module is loaded in isolated mode)
    std::unordered_map<std::string, std::unordered_map<std::string, Value>> moduleExportCache_;
    // Cache: canonical path -> parsed program (keeps AST alive for UserFunction closures)
    std::unordered_map<std::string, std::vector<StmtPtr>> moduleProgramCache_;

    // command line arguments for std.process
    std::vector<std::string> commandLineArgs;

    // helpers
    std::string loadFile(const std::string& path);
    std::string normalizeModulePath(const std::string& path);
    // Load a file module in isolated scope, collect exports, cache and return them
    std::unordered_map<std::string, Value> loadModuleWithExports(const std::string& modulePath,
                                                                  const std::string& fromFile);

   public:
    // Set current file for relative import resolution
    void setCurrentFile(const std::string& filename) { currentFile = filename; }
    const std::string& getCurrentFile() const { return currentFile; }
};

}  // namespace izi

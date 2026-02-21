#pragma once

#include "ast/stmt.hpp"
#include "bytecode/opcode.hpp"
#include "bytecode/chunk.hpp"
#include <unordered_set>
#include <string>

namespace izi {
class BytecodeCompiler : public ExprVisitor, public StmtVisitor {
   public:
    BytecodeCompiler() = default;
    Chunk compile(const std::vector<StmtPtr>& program);

    // Set the imported modules cache (shared across compiler instances)
    void setImportedModules(std::unordered_set<std::string>* modules) { importedModules = modules; }

    // Expression visitors
    auto visit(BinaryExpr& expr) -> Value override;
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
    Value visit(PropertyExpr& expr) override;  // v0.3
    Value visit(SetPropertyExpr& expr) override;  // v0.3
    Value visit(ThisExpr& expr) override;  // v0.3
    Value visit(SuperExpr& expr) override;  // v0.3
    Value visit(AwaitExpr& expr) override;  // async/await

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
    void visit(ReExportStmt& stmt) override;
    void visit(BreakStmt& stmt) override;
    void visit(ContinueStmt& stmt) override;
    void visit(TryStmt& stmt) override;
    void visit(ThrowStmt& stmt) override;
    void visit(ClassStmt& stmt) override;  // v0.3

   private:
    Chunk chunk;
    std::unordered_set<std::string>* importedModules = nullptr;
    std::vector<std::string> importStack;  // Track files being imported (for circular detection)
    std::string currentFile;  // Current file being compiled

    // Register allocation: local variable slots for function scopes.
    // When non-empty, the compiler is inside a function body and uses
    // GET_LOCAL / SET_LOCAL instead of GET_GLOBAL / SET_GLOBAL for
    // variables whose names appear in this table.
    std::vector<std::string> locals;  // locals[i] = name of local at slot i

    // Loop context for break/continue
    struct LoopContext {
        std::vector<size_t> breakJumps;
        size_t loopStart;
    };
    std::vector<LoopContext> loopStack;

    void emitByte(uint8_t byte) { chunk.write(byte); }
    void emitOp(OpCode op) { emitByte(static_cast<uint8_t>(op)); }

    uint8_t makeConstant(const Value& b);
    uint8_t makeName(const std::string& name);

    void emitExpression(Expr& expr);
    void emitStatement(Stmt& stmt);

    // control flow helpers
    size_t emitJump(OpCode op);
    void patchJump(size_t offset);
    void emitLoop(size_t loopStart);

    // Register allocation helpers
    // Returns the local slot index for the given name, or -1 if not a local.
    int resolveLocal(const std::string& name) const;

    // Import helpers
    std::string normalizeModulePath(const std::string& path);
    std::string loadFile(const std::string& path);

   public:
    // Set current file for relative import resolution
    void setCurrentFile(const std::string& filename) { currentFile = filename; }
    const std::string& getCurrentFile() const { return currentFile; }
};
}  // namespace izi

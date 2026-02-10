#pragma once

#include "ast/stmt.hpp"
#include "bytecode/opcode.hpp"
#include "bytecode/chunk.hpp"
#include <unordered_set>
#include <string>



namespace izi
{
    class BytecodeCompiler: public ExprVisitor, public StmtVisitor
    {
    public:
        BytecodeCompiler() = default;
        Chunk compile(const std::vector<StmtPtr>& program);
        
        // Set the imported modules cache (shared across compiler instances)
        void setImportedModules(std::unordered_set<std::string>* modules) {
            importedModules = modules;
        }

        // Expression visitors
        auto visit(BinaryExpr& expr) -> Value override;
        Value visit(UnaryExpr& expr) override;
        Value visit(CallExpr& expr) override;
        Value visit(LiteralExpr& expr) override;
        Value visit(AssignExpr& expr) override;
        Value visit(VariableExpr& expr) override;
        Value visit(GroupingExpr& expr) override;
        Value visit(ArrayExpr& expr) override;
        Value visit(MapExpr& expr) override;
        Value visit(IndexExpr& expr) override;
        Value visit(SetIndexExpr& expr) override;

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

    private:
        Chunk chunk;
        std::unordered_set<std::string>* importedModules = nullptr;
        
        void emitByte(uint8_t byte){chunk.write(byte); }
        void emitOp(OpCode op) { emitByte(static_cast<uint8_t>(op)); }

        uint8_t makeConstant(const Value& b);
        uint8_t makeName(const std::string& name);


        void emitExpression(Expr& expr);
        void emitStatement(Stmt& stmt);

        // control flow helpers
        size_t emitJump(OpCode op);
        void patchJump(size_t offset);
        void emitLoop(size_t loopStart);
        
        // Import helpers
        std::string normalizeModulePath(const std::string& path);
        std::string loadFile(const std::string& path);

    };
} // namespace izi

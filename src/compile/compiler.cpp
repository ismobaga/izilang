#include "compiler.hpp"
#include "common/value.hpp"
#include "bytecode/vm_user_function.hpp"
#include <stdexcept>

namespace izi {
Chunk BytecodeCompiler::compile(const std::vector<StmtPtr>& program) {
    for (const auto& stmt : program) {
        emitStatement(*stmt);
    }

    // Emit final return
    emitOp(OpCode::NIL);
    emitOp(OpCode::RETURN);
    return std::move(chunk);
}

uint8_t BytecodeCompiler::makeConstant(const Value& value) {
    size_t index = chunk.addConstant(value);
    if (index > UINT8_MAX) {
        throw std::runtime_error("Too many constants in one chunk.");
    }
    return static_cast<uint8_t>(index);
}

uint8_t BytecodeCompiler::makeName(const std::string& name) {
    size_t index = chunk.addName(name);
    if (index > UINT8_MAX) {
        throw std::runtime_error("Too many names in one chunk.");
    }
    return static_cast<uint8_t>(index);
}
void BytecodeCompiler::emitExpression(Expr& expr) {
    expr.accept(*this);
}
void BytecodeCompiler::emitStatement(Stmt& stmt) {
    stmt.accept(*this);    
}

// control flow helpers
size_t BytecodeCompiler::emitJump(OpCode op) {
    emitOp(op);
    // Placeholder for jump offset
    emitByte(0xff);
    emitByte(0xff);
    return chunk.code.size() - 2;   
}

void BytecodeCompiler::patchJump(size_t offset) {
    size_t jumpOffset = chunk.code.size() - offset - 2;
    if (jumpOffset > UINT16_MAX) {
        throw std::runtime_error("Too much code to jump over.");
    }
    chunk.code[offset] = (jumpOffset >> 8) & 0xff;
    chunk.code[offset + 1] = jumpOffset & 0xff;
}

void BytecodeCompiler::emitLoop(size_t loopStart) {
    emitOp(OpCode::LOOP);
    size_t offset = chunk.code.size() - loopStart + 2;
    if (offset > UINT16_MAX) {
        throw std::runtime_error("Loop body too large.");
    }
    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}


//  --- ExprVisitor
Value BytecodeCompiler::visit(BinaryExpr& expr) {
    emitExpression(*expr.left);
    emitExpression(*expr.right);

    switch (expr.op.type) {
        case TokenType::PLUS:
            emitOp(OpCode::ADD);
            break;
        case TokenType::MINUS:
            emitOp(OpCode::SUBTRACT);
            break;
        case TokenType::STAR:
            emitOp(OpCode::MULTIPLY);
            break;
        case TokenType::SLASH:
            emitOp(OpCode::DIVIDE);
            break;
        case TokenType::GREATER:
            emitOp(OpCode::GREATER);
            break;
        case TokenType::GREATER_EQUAL:
            emitOp(OpCode::GREATER_EQUAL);
            break;
        case TokenType::LESS:
            emitOp(OpCode::LESS);
            break;
        case TokenType::LESS_EQUAL:
            emitOp(OpCode::LESS_EQUAL);
            break;
        case TokenType::EQUAL_EQUAL:
            emitOp(OpCode::EQUAL);
            break;
        case TokenType::BANG_EQUAL:
            emitOp(OpCode::NOT_EQUAL);
            break;
        default:
            throw std::runtime_error("Unknown binary operator.");
    }

    return Nil{};

}

Value BytecodeCompiler::visit(UnaryExpr& expr) {
    emitExpression(*expr.right);

    switch (expr.op.type) {
        case TokenType::MINUS:
            emitOp(OpCode::NEGATE);
            break;
        case TokenType::BANG:
            emitOp(OpCode::NOT);
            break;
        default:
            throw std::runtime_error("Unknown unary operator.");
    }

    return Nil{};
}

Value BytecodeCompiler::visit(CallExpr& expr) {
    emitExpression(*expr.callee);
    for (const auto& arg : expr.args) {
        emitExpression(*arg);
    }
    emitOp(OpCode::CALL);
    emitByte(static_cast<uint8_t>(expr.args.size()));
    return Nil{};

}

Value BytecodeCompiler::visit(LiteralExpr& expr) {
    if (std::holds_alternative<Nil>(expr.value)) {
        emitOp(OpCode::NIL);
    } else if (std::holds_alternative<bool>(expr.value)) {
        if (std::get<bool>(expr.value)) {
            emitOp(OpCode::TRUE);
        } else {
            emitOp(OpCode::FALSE);
        }
    } else {
        uint8_t constIndex = makeConstant(expr.value);
        emitOp(OpCode::CONSTANT);
        emitByte(constIndex);
    }
    return Nil{};
}

Value BytecodeCompiler::visit(AssignExpr& expr) {
    emitExpression(*expr.value);
    uint8_t nameIndex = makeName(expr.name);
    emitOp(OpCode::SET_GLOBAL);
    emitByte(nameIndex);
    return Nil{};
}

Value BytecodeCompiler::visit(VariableExpr& expr) {
    uint8_t nameIndex = makeName(expr.name);
    emitOp(OpCode::GET_GLOBAL);
    emitByte(nameIndex);
    return Nil{};
}
Value BytecodeCompiler::visit(GroupingExpr& expr) {
    emitExpression(*expr.expression);
    return Nil{};
}

Value BytecodeCompiler::visit(IndexExpr& expr) {
    emitExpression(*expr.collection);
    emitExpression(*expr.index);
    emitOp(OpCode::INDEX);
    return Nil{};
}
Value BytecodeCompiler::visit(SetIndexExpr& expr) {
    emitExpression(*expr.collection);
    emitExpression(*expr.index);
    emitExpression(*expr.value);
    emitOp(OpCode::SET_INDEX);
    return Nil{};
}
Value BytecodeCompiler::visit(ArrayExpr& expr) {
    for (const auto& element : expr.elements) {
        emitExpression(*element);
    }
    uint8_t count = static_cast<uint8_t>(expr.elements.size());
    emitOp(OpCode::CONSTANT);
    emitByte(makeConstant(static_cast<double>(count))); // Store count as constant
    return Nil{};
}

Value BytecodeCompiler::visit(MapExpr& expr) {
    for (const auto& [key, valueExpr] : expr.entries) {
        emitExpression(*valueExpr);
        emitOp(OpCode::CONSTANT);
        emitByte(makeConstant(key)); // Store key as constant
    }
    uint8_t count = static_cast<uint8_t>(expr.entries.size());
    emitOp(OpCode::CONSTANT);
    emitByte(makeConstant(static_cast<double>(count))); // Store count as constant
    return Nil{};

}
//  --- StmtVisitor
void BytecodeCompiler::visit(ExprStmt& stmt) {
    emitExpression(*stmt.expr);
    emitOp(OpCode::POP);
}
void BytecodeCompiler::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        emitExpression(*stmt.value);
    } else {
        emitOp(OpCode::NIL);
    }
    emitOp(OpCode::RETURN);
}

void BytecodeCompiler::visit(IfStmt& stmt) {
    emitExpression(*stmt.condition);
    size_t thenJump = emitJump(OpCode::JUMP_IF_FALSE);
    emitOp(OpCode::POP); // Pop condition

    emitStatement(*stmt.thenBranch);
    size_t elseJump = emitJump(OpCode::JUMP);

    patchJump(thenJump);
    emitOp(OpCode::POP); // Pop condition

    if (stmt.elseBranch) {
        emitStatement(*stmt.elseBranch);
    }
    patchJump(elseJump);
}
 void BytecodeCompiler::visit(WhileStmt& stmt) {
    size_t loopStart = chunk.code.size();
    emitExpression(*stmt.condition);

    size_t exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    emitOp(OpCode::POP); // Pop condition

    emitStatement(*stmt.body);
    emitLoop(loopStart);

    patchJump(exitJump);
    emitOp(OpCode::POP); // Pop condition

 }
void BytecodeCompiler::visit(BlockStmt& stmt) {
    for (const auto& statement : stmt.statements) {
        emitStatement(*statement);
    }

}
void BytecodeCompiler::visit(VarStmt& stmt) {
    Value initValue = Nil{};
    if (stmt.initializer) {
        emitExpression(*stmt.initializer);
    } else {
        emitOp(OpCode::NIL);
    }
    uint8_t nameIndex = makeName(stmt.name);
    emitOp(OpCode::SET_GLOBAL);
    emitByte(nameIndex);
}

void BytecodeCompiler::visit(FunctionStmt& stmt) {
    // Compile function body into a separate chunk
    BytecodeCompiler functionCompiler;
    
    // Compile the function body
    for (const auto& bodyStmt : stmt.body) {
        functionCompiler.emitStatement(*bodyStmt);
    }
    
    // Ensure the function returns nil if it doesn't have an explicit return
    functionCompiler.emitOp(OpCode::NIL);
    functionCompiler.emitOp(OpCode::RETURN);
    
    // Create a VmUserFunction and store it as a constant
    auto functionChunk = std::make_shared<Chunk>(std::move(functionCompiler.chunk));
    auto vmFunction = std::make_shared<VmUserFunction>(
        stmt.name,
        stmt.params,
        functionChunk
    );
    
    // Store the function in a constant
    uint8_t constantIndex = makeConstant(vmFunction);
    emitOp(OpCode::CONSTANT);
    emitByte(constantIndex);
    
    // Store it in a global variable
    uint8_t nameIndex = makeName(stmt.name);
    emitOp(OpCode::SET_GLOBAL);
    emitByte(nameIndex);
}
void BytecodeCompiler::visit(ImportStmt& stmt) {
    // Import compilation not implemented yet
    throw std::runtime_error("Import compilation not implemented.");
}




}
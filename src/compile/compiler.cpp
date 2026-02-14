#include "compiler.hpp"
#include "common/value.hpp"
#include "common/module_path.hpp"
#include "bytecode/vm_user_function.hpp"
#include "bytecode/vm_class.hpp"
#include "bytecode/vm_native_modules.hpp"
#include "bytecode/chunk_serializer.hpp"
#include "bytecode/opcode.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

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

Value BytecodeCompiler::visit(FunctionExpr& expr) {
    // For now, we don't support function expressions in bytecode mode
    // This would require compiling the function body to bytecode and creating
    // a VmCallable at runtime
    throw std::runtime_error("Function expressions are not yet supported in bytecode mode.");
}

Value BytecodeCompiler::visit(MatchExpr& expr) {
    // For now, we don't support match expressions in bytecode mode
    // This would require compiling pattern matching logic to bytecode
    throw std::runtime_error("Match expressions are not yet supported in bytecode mode.");
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

Value BytecodeCompiler::visit(SpreadExpr& expr) {
    // For now, just compile the argument
    // The actual spread logic should be handled at runtime in the VM
    emitExpression(*expr.argument);
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
    
    // Push new loop context for break/continue
    loopStack.push_back(LoopContext{{}, loopStart});
    
    emitExpression(*stmt.condition);

    size_t exitJump = emitJump(OpCode::JUMP_IF_FALSE);
    emitOp(OpCode::POP); // Pop condition

    emitStatement(*stmt.body);
    emitLoop(loopStart);

    patchJump(exitJump);
    emitOp(OpCode::POP); // Pop condition
    
    // Patch all break jumps to exit the loop
    for (size_t breakJump : loopStack.back().breakJumps) {
        patchJump(breakJump);
    }
    
    // Pop loop context
    loopStack.pop_back();
 }
void BytecodeCompiler::visit(BlockStmt& stmt) {
    for (const auto& statement : stmt.statements) {
        emitStatement(*statement);
    }

}
void BytecodeCompiler::visit(VarStmt& stmt) {
    // For now, bytecode compiler doesn't support destructuring
    // Just handle simple variable declarations
    if (stmt.pattern != nullptr) {
        throw std::runtime_error("Bytecode compiler does not yet support destructuring syntax.");
    }
    
    Value initValue = Nil{};
    if (stmt.initializer) {
        emitExpression(*stmt.initializer);
    } else {
        emitOp(OpCode::NIL);
    }
    uint8_t nameIndex = makeName(stmt.name);
    emitOp(OpCode::SET_GLOBAL);
    emitByte(nameIndex);
    emitOp(OpCode::POP); // Pop the value left by SET_GLOBAL
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
    emitOp(OpCode::POP); // Pop the value left by SET_GLOBAL
}

void BytecodeCompiler::visit(ImportStmt& stmt) {
    std::string modulePath = stmt.module;  // Use original module name first
    
    // Check if this is a native module - if so, skip compilation
    if (isVmNativeModule(modulePath)) {
        // Native modules are registered at VM initialization
        // Mark as imported to prevent duplicate processing
        if (importedModules) {
            importedModules->insert(modulePath);
        }
        return;
    }
    
    // For file-based modules, resolve relative paths
    // Note: We handle extension resolution ourselves to support .izb files
    std::string basePath = stmt.module;
    
    // If it's a relative path, resolve it relative to the importing file's directory
    if (ModulePath::isRelativePath(basePath)) {
        std::string importingDir = ModulePath::getDirectory(currentFile);
        std::filesystem::path dirPath(importingDir);
        std::filesystem::path modulePath = dirPath / basePath;
        basePath = modulePath.lexically_normal().string();
    }
    
    // Check if we need to try .izb extension first
    std::string actualPath = basePath;
    bool isPrecompiled = false;
    
    // If path doesn't have an extension, try .izb first, then .izi
    // Note: We support both .iz and .izi extensions for compatibility
    // .izi is the canonical extension, but .iz is also widely used
    if (!basePath.ends_with(".iz") && !basePath.ends_with(".izb") && !basePath.ends_with(".izi")) {
        std::string izbPath = basePath + ".izb";
        std::ifstream izbFile(izbPath);
        if (izbFile.good()) {
            actualPath = izbPath;
            isPrecompiled = true;
        } else {
            // Fall back to .izi extension (canonical)
            actualPath = basePath + ".izi";
        }
    } else if (basePath.ends_with(".izb")) {
        isPrecompiled = true;
        actualPath = basePath;
    } else {
        // .iz or .izi - both are source files
        actualPath = basePath;
    }
    
    // Canonicalize the path for proper deduplication and cycle detection
    std::string canonicalPath = ModulePath::canonicalize(actualPath);
    
    // Check if module is already imported (avoid re-importing)
    if (importedModules && importedModules->contains(canonicalPath)) {
        return;
    }
    
    // Check for circular imports
    auto it = std::find(importStack.begin(), importStack.end(), canonicalPath);
    if (it != importStack.end()) {
        // Build circular import chain message
        std::string chain;
        for (size_t i = std::distance(importStack.begin(), it); i < importStack.size(); ++i) {
            if (!chain.empty()) chain += " -> ";
            chain += importStack[i];
        }
        chain += " -> " + canonicalPath;
        throw std::runtime_error("Circular import detected: " + chain);
    }
    
    // Push to import stack
    importStack.push_back(canonicalPath);
    std::string previousFile = currentFile;
    currentFile = canonicalPath;
    
    try {
        if (isPrecompiled) {
            // Load precompiled bytecode chunk
            Chunk moduleChunk = ChunkSerializer::deserializeFromFile(actualPath);
            
            // We need to remap constant and name indices as we merge chunks
            size_t constantOffset = chunk.constants.size();
            size_t nameOffset = chunk.names.size();
            
            // Check for overflow before merging
            if (constantOffset + moduleChunk.constants.size() > 255) {
                throw std::runtime_error("Too many constants when importing module (limit: 256 per chunk)");
            }
            if (nameOffset + moduleChunk.names.size() > 255) {
                throw std::runtime_error("Too many names when importing module (limit: 256 per chunk)");
            }
            
            // Merge constants
            for (const auto& constant : moduleChunk.constants) {
                chunk.constants.push_back(constant);
            }
            
            // Merge names
            for (const auto& name : moduleChunk.names) {
                chunk.names.push_back(name);
            }
            
            // Copy and remap bytecode, but skip the final NIL+RETURN that was added during compilation
            // The final NIL+RETURN is always the last 2 bytes in a compiled chunk
            size_t codeEndPos = moduleChunk.code.size();
            if (codeEndPos >= 2 && 
                static_cast<OpCode>(moduleChunk.code[codeEndPos - 2]) == OpCode::NIL &&
                static_cast<OpCode>(moduleChunk.code[codeEndPos - 1]) == OpCode::RETURN) {
                codeEndPos -= 2;  // Skip the final NIL+RETURN
            }
            
            for (size_t i = 0; i < codeEndPos; ++i) {
                uint8_t byte = moduleChunk.code[i];
                OpCode op = static_cast<OpCode>(byte);
                
                chunk.code.push_back(byte);
                
                // Check if this opcode is followed by a constant or name index
                if (op == OpCode::CONSTANT) {
                    // Next byte is a constant index
                    if (i + 1 < moduleChunk.code.size()) {
                        ++i;
                        uint8_t constIndex = moduleChunk.code[i];
                        uint8_t remappedIndex = static_cast<uint8_t>(constIndex + constantOffset);
                        chunk.code.push_back(remappedIndex);
                    }
                } else if (op == OpCode::GET_GLOBAL || op == OpCode::SET_GLOBAL || 
                          op == OpCode::GET_PROPERTY || op == OpCode::SET_PROPERTY) {
                    // Next byte is a name index
                    if (i + 1 < moduleChunk.code.size()) {
                        ++i;
                        uint8_t nameIndex = moduleChunk.code[i];
                        uint8_t remappedIndex = static_cast<uint8_t>(nameIndex + nameOffset);
                        chunk.code.push_back(remappedIndex);
                    }
                } else if (op == OpCode::JUMP || op == OpCode::JUMP_IF_FALSE || op == OpCode::LOOP) {
                    // Next 2 bytes are jump offset (no remapping needed, relative offset)
                    if (i + 2 < moduleChunk.code.size()) {
                        ++i;
                        chunk.code.push_back(moduleChunk.code[i]);
                        ++i;
                        chunk.code.push_back(moduleChunk.code[i]);
                    }
                } else if (op == OpCode::CALL) {
                    // Next byte is argument count (no remapping needed)
                    if (i + 1 < moduleChunk.code.size()) {
                        ++i;
                        chunk.code.push_back(moduleChunk.code[i]);
                    }
                } else if (op == OpCode::TRY) {
                    // Next 5 bytes: catch offset (2), finally offset (2), catch var name index (1)
                    if (i + 5 < moduleChunk.code.size()) {
                        ++i; chunk.code.push_back(moduleChunk.code[i]); // catch offset high
                        ++i; chunk.code.push_back(moduleChunk.code[i]); // catch offset low
                        ++i; chunk.code.push_back(moduleChunk.code[i]); // finally offset high
                        ++i; chunk.code.push_back(moduleChunk.code[i]); // finally offset low
                        ++i;
                        uint8_t catchVarIndex = moduleChunk.code[i];
                        if (catchVarIndex != 0) {
                            // Remap catch variable name index
                            catchVarIndex = static_cast<uint8_t>(catchVarIndex + nameOffset);
                        }
                        chunk.code.push_back(catchVarIndex);
                    }
                }
            }
        } else {
            // Load and parse the module source
            std::string source = loadFile(actualPath);
            Lexer lexer(source);
            auto tokens = lexer.scanTokens();
            Parser parser(std::move(tokens));
            auto program = parser.parse();
            
            // Compile the module's statements inline
            for (const auto& moduleStmt : program) {
                emitStatement(*moduleStmt);
            }
        }
        
        // Pop from import stack and mark as imported
        importStack.pop_back();
        if (importedModules) {
            importedModules->insert(canonicalPath);
        }
        
        // Restore previous file
        currentFile = previousFile;
    } catch (...) {
        // On error, restore state and rethrow
        importStack.pop_back();
        currentFile = previousFile;
        throw;
    }
}

void BytecodeCompiler::visit(ExportStmt& stmt) {
    // For now, simply emit the underlying declaration
    // The declaration (function or variable) will be defined globally
    // In a future enhancement, we could track exported names for validation
    emitStatement(*stmt.declaration);
}

void BytecodeCompiler::visit(BreakStmt& /*stmt*/) {
    if (loopStack.empty()) {
        throw std::runtime_error("'break' statement outside of loop.");
    }
    
    // Record this jump to be patched when we exit the loop
    size_t breakJump = emitJump(OpCode::JUMP);
    loopStack.back().breakJumps.push_back(breakJump);
}

void BytecodeCompiler::visit(ContinueStmt& /*stmt*/) {
    if (loopStack.empty()) {
        throw std::runtime_error("'continue' statement outside of loop.");
    }
    
    // Emit a LOOP instruction to jump back to loop start
    emitLoop(loopStack.back().loopStart);
}

void BytecodeCompiler::visit(TryStmt& stmt) {
    // Emit TRY opcode with placeholders for offsets
    size_t tryOpcodePos = chunk.code.size();
    emitOp(OpCode::TRY);
    size_t catchOffsetPos = chunk.code.size();
    emitByte(0xff);  // Placeholder for catch offset high byte
    emitByte(0xff);  // Placeholder for catch offset low byte
    size_t finallyOffsetPos = chunk.code.size();
    emitByte(0xff);  // Placeholder for finally offset high byte
    emitByte(0xff);  // Placeholder for finally offset low byte
    
    // Emit catch variable name index (0 if no catch)
    uint8_t catchVarIndex = 0;
    if (stmt.catchBlock != nullptr && !stmt.catchVariable.empty()) {
        catchVarIndex = makeName(stmt.catchVariable);
    }
    emitByte(catchVarIndex);
    
    // Now IP is at tryOpcodePos + 6 (1 opcode + 2 + 2 + 1 bytes)
    size_t ipAfterTry = chunk.code.size();
    
    // Emit try block
    emitStatement(*stmt.tryBlock);
    
    // Jump over catch and finally blocks after successful try execution
    size_t tryEndJump = emitJump(OpCode::JUMP);
    
    // Emit catch block if present
    if (stmt.catchBlock != nullptr) {
        size_t catchStart = chunk.code.size();
        uint16_t catchOffset = catchStart - ipAfterTry;
        chunk.code[catchOffsetPos] = (catchOffset >> 8) & 0xff;
        chunk.code[catchOffsetPos + 1] = catchOffset & 0xff;
        
        // Pop exception from stack - it was pushed by the exception handler
        // and bound to the catch variable as a global
        emitOp(OpCode::POP);
        
        // Emit catch block body
        emitStatement(*stmt.catchBlock);
        
        // If there's no finally, jump to after the finally block
        if (stmt.finallyBlock == nullptr) {
            size_t catchEndJump = emitJump(OpCode::JUMP);
            patchJump(tryEndJump);
            patchJump(catchEndJump);
        } else {
            // Jump to finally block
            size_t catchToFinallyJump = emitJump(OpCode::JUMP);
            patchJump(tryEndJump);
            
            // Emit finally block
            size_t finallyStart = chunk.code.size();
            uint16_t finallyOffset = finallyStart - ipAfterTry;
            chunk.code[finallyOffsetPos] = (finallyOffset >> 8) & 0xff;
            chunk.code[finallyOffsetPos + 1] = finallyOffset & 0xff;
            
            emitStatement(*stmt.finallyBlock);
            
            // Patch the jumps to finally
            patchJump(catchToFinallyJump);
        }
    } else if (stmt.finallyBlock != nullptr) {
        // No catch, but there's finally
        // Set catch offset to 0 (no catch)
        chunk.code[catchOffsetPos] = 0;
        chunk.code[catchOffsetPos + 1] = 0;
        
        patchJump(tryEndJump);
        
        // Emit finally block
        size_t finallyStart = chunk.code.size();
        uint16_t finallyOffset = finallyStart - ipAfterTry;
        chunk.code[finallyOffsetPos] = (finallyOffset >> 8) & 0xff;
        chunk.code[finallyOffsetPos + 1] = finallyOffset & 0xff;
        
        emitStatement(*stmt.finallyBlock);
    } else {
        // No catch or finally - just patch the try end jump
        chunk.code[catchOffsetPos] = 0;
        chunk.code[catchOffsetPos + 1] = 0;
        chunk.code[finallyOffsetPos] = 0;
        chunk.code[finallyOffsetPos + 1] = 0;
        patchJump(tryEndJump);
    }
    
    // Emit END_TRY to clean up the exception handler
    emitOp(OpCode::END_TRY);
}

void BytecodeCompiler::visit(ThrowStmt& stmt) {
    // Evaluate the expression to throw
    emitExpression(*stmt.value);
    
    // Emit THROW opcode
    emitOp(OpCode::THROW);
}

std::string BytecodeCompiler::normalizeModulePath(const std::string& path) {
    // Turn "math" into "math.iz"
    if (path.size() >= 3 && path.ends_with(".iz")) {
        return path;
    }
    return path + ".iz";
}

std::string BytecodeCompiler::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// v0.3: Class support
void BytecodeCompiler::visit(ClassStmt& stmt) {
    // For a class, we need to:
    // 1. Compile each method into a separate function
    // 2. Create a VmClass with those methods
    // 3. Store the class as a global
    
    std::unordered_map<std::string, std::shared_ptr<VmCallable>> methods;
    std::vector<std::string> fieldNames;
    std::unordered_map<std::string, Value> fieldDefaults;
    
    // Compile methods
    for (const auto& method : stmt.methods) {
        BytecodeCompiler methodCompiler;
        
        // Compile the method body
        for (const auto& bodyStmt : method->body) {
            methodCompiler.emitStatement(*bodyStmt);
        }
        
        // Ensure the method returns nil if it doesn't have an explicit return
        methodCompiler.emitOp(OpCode::NIL);
        methodCompiler.emitOp(OpCode::RETURN);
        
        // Create a VmUserFunction for the method
        auto methodChunk = std::make_shared<Chunk>(std::move(methodCompiler.chunk));
        auto vmMethod = std::make_shared<VmUserFunction>(
            method->name,
            method->params,
            methodChunk
        );
        
        methods[method->name] = vmMethod;
    }
    
    // Extract field names and defaults
    // Note: Field initializers are not yet supported in bytecode compilation
    // All fields are initialized to nil; initializers should be set in constructors
    for (const auto& field : stmt.fields) {
        fieldNames.push_back(field->name);
        fieldDefaults[field->name] = Nil{};
    }
    
    // Create the VmClass
    auto vmClass = std::make_shared<VmClass>(
        stmt.name,
        fieldNames,
        fieldDefaults,
        methods
    );
    
    // Store the class as a constant
    uint8_t constantIndex = makeConstant(vmClass);
    emitOp(OpCode::CONSTANT);
    emitByte(constantIndex);
    
    // Store it in a global variable with the class name
    uint8_t nameIndex = makeName(stmt.name);
    emitOp(OpCode::SET_GLOBAL);
    emitByte(nameIndex);
    emitOp(OpCode::POP); // Pop the value left by SET_GLOBAL
}

// v0.3: Property access
Value BytecodeCompiler::visit(PropertyExpr& expr) {
    // Compile the object expression
    emitExpression(*expr.object);
    
    // Emit GET_PROPERTY with the property name
    uint8_t nameIndex = makeName(expr.property);
    emitOp(OpCode::GET_PROPERTY);
    emitByte(nameIndex);
    
    return Nil{}; // Return value not used in visitor pattern
}

// v0.3: Property assignment
Value BytecodeCompiler::visit(SetPropertyExpr& expr) {
    // Compile the object expression
    emitExpression(*expr.object);
    
    // Compile the value expression
    emitExpression(*expr.value);
    
    // Emit SET_PROPERTY with the property name
    uint8_t nameIndex = makeName(expr.property);
    emitOp(OpCode::SET_PROPERTY);
    emitByte(nameIndex);
    
    return Nil{}; // Return value not used in visitor pattern
}

// v0.3: This expression
Value BytecodeCompiler::visit(ThisExpr& expr) {
    // 'this' is implemented as a global variable lookup
    // Note: This is a simplified implementation. It works for basic use cases
    // but has limitations with nested method calls or recursion. See VmBoundMethod
    // for details. A proper implementation would use local variables or call frames.
    uint8_t nameIndex = makeName("this");
    emitOp(OpCode::GET_GLOBAL);
    emitByte(nameIndex);
    
    return Nil{}; // Return value not used in visitor pattern
}

// v0.3: Super expression
Value BytecodeCompiler::visit(SuperExpr& expr) {
    // For bytecode VM, we implement super as:
    // 1. Load 'super' (the superclass) - set in method closure during class creation
    // 2. Load 'this' (the current instance) - bound when method is called
    // 3. Get the method from the superclass and bind it to this
    // Note: Both 'super' and 'this' are treated as globals in the simplified VM implementation
    
    // Load super
    uint8_t superIndex = makeName("super");
    emitOp(OpCode::GET_GLOBAL);
    emitByte(superIndex);
    
    // Load this
    uint8_t thisIndex = makeName("this");
    emitOp(OpCode::GET_GLOBAL);
    emitByte(thisIndex);
    
    // Get and bind the method
    uint8_t methodIndex = makeName(expr.method);
    emitOp(OpCode::GET_SUPER_METHOD);
    emitByte(methodIndex);
    
    return Nil{}; // Return value not used in visitor pattern
}

}
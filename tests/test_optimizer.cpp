#include "catch.hpp"
#include "compile/optimizer.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "bytecode/vm_user_function.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

// Helper function to parse code
std::vector<StmtPtr> parseCode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), code);
    return parser.parse();
}

TEST_CASE("Optimizer: Constant folding for binary expressions", "[optimizer][constant-folding]") {
    SECTION("Addition") {
        auto program = parseCode("var x = 2 + 3;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        REQUIRE(optimized.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "x");

        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(literal != nullptr);
        REQUIRE(std::holds_alternative<double>(literal->value));
        REQUIRE(std::get<double>(literal->value) == 5.0);
    }

    SECTION("Subtraction") {
        auto program = parseCode("var x = 10 - 4;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == 6.0);
    }

    SECTION("Multiplication") {
        auto program = parseCode("var x = 5 * 6;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == 30.0);
    }

    SECTION("Division") {
        auto program = parseCode("var x = 20 / 4;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == 5.0);
    }

    SECTION("Comparison operators") {
        auto program = parseCode("var x = 5 > 3;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::holds_alternative<bool>(literal->value));
        REQUIRE(std::get<bool>(literal->value) == true);
    }
}

TEST_CASE("Optimizer: Constant folding for unary expressions", "[optimizer][constant-folding]") {
    SECTION("Negation") {
        auto program = parseCode("var x = -5;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == -5.0);
    }

    SECTION("Logical NOT") {
        auto program = parseCode("var x = !false;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::holds_alternative<bool>(literal->value));
        REQUIRE(std::get<bool>(literal->value) == true);
    }
}

TEST_CASE("Optimizer: Dead code elimination after return", "[optimizer][dead-code]") {
    auto program = parseCode(R"(
        fn test() {
            print("before");
            return 42;
            print("after");
        }
    )");

    Optimizer optimizer;
    auto optimized = optimizer.optimize(std::move(program));

    REQUIRE(optimized.size() == 1);
    auto* funcStmt = dynamic_cast<FunctionStmt*>(optimized[0].get());
    REQUIRE(funcStmt != nullptr);

    // The function body should have 2 statements, not 3
    // (print and return, but not the unreachable print)
    REQUIRE(funcStmt->body.size() == 2);
}

TEST_CASE("Optimizer: Dead code elimination in if statements", "[optimizer][dead-code]") {
    SECTION("Constant true condition") {
        auto program = parseCode(R"(
            if (true) {
                print("then");
            } else {
                print("else");
            }
        )");

        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        REQUIRE(optimized.size() == 1);
        // The if statement should be replaced with just the then branch
        auto* blockStmt = dynamic_cast<BlockStmt*>(optimized[0].get());
        REQUIRE(blockStmt != nullptr);
    }

    SECTION("Constant false condition") {
        auto program = parseCode(R"(
            if (false) {
                print("then");
            } else {
                print("else");
            }
        )");

        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));

        REQUIRE(optimized.size() == 1);
        // The if statement should be replaced with the else branch
        auto* blockStmt = dynamic_cast<BlockStmt*>(optimized[0].get());
        REQUIRE(blockStmt != nullptr);
    }
}

TEST_CASE("Optimizer: Dead code elimination in while loops", "[optimizer][dead-code]") {
    auto program = parseCode(R"(
        while (false) {
            print("never");
        }
    )");

    Optimizer optimizer;
    auto optimized = optimizer.optimize(std::move(program));

    // The while loop with a constant false condition should be eliminated
    REQUIRE(optimized.size() == 0);
}

TEST_CASE("Optimizer: Non-constant expressions are not folded", "[optimizer]") {
    auto program = parseCode("var x = a + b;");
    Optimizer optimizer;
    auto optimized = optimizer.optimize(std::move(program));

    auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
    REQUIRE(varStmt != nullptr);

    // Should still be a binary expression, not a literal
    auto* binaryExpr = dynamic_cast<BinaryExpr*>(varStmt->initializer.get());
    REQUIRE(binaryExpr != nullptr);
}

// ---------------------------------------------------------------------------
// Register allocation tests
// ---------------------------------------------------------------------------
// These tests verify that function parameters are allocated to local variable
// slots (GET_LOCAL / SET_LOCAL opcodes) instead of the global hash-map, and
// that functions execute correctly with the register-allocated parameters.
// ---------------------------------------------------------------------------

TEST_CASE("Register allocation: function parameters use GET_LOCAL", "[optimizer][register-alloc]") {
    SECTION("Function body emits GET_LOCAL for parameter access") {
        // Compile a simple function and inspect the emitted bytecode.
        // The function has one parameter 'n'; inside the body 'n' should be
        // accessed via GET_LOCAL 0 (not GET_GLOBAL).
        auto program = parseCode("fn double(n) { return n + n; }");
        BytecodeCompiler compiler;
        auto chunk = compiler.compile(program);

        // The function constant is stored in chunk.constants[0].
        // Verify the function chunk uses GET_LOCAL, not GET_GLOBAL.
        REQUIRE(!chunk.constants.empty());
        auto& fnVal = chunk.constants[0];
        REQUIRE(std::holds_alternative<std::shared_ptr<VmCallable>>(fnVal));
        auto callable = std::get<std::shared_ptr<VmCallable>>(fnVal);
        auto* userFn = dynamic_cast<VmUserFunction*>(callable.get());
        REQUIRE(userFn != nullptr);

        const auto& fnChunk = userFn->getChunk();
        bool hasGetLocal = false;
        bool hasGetGlobalForParam = false;
        for (size_t i = 0; i < fnChunk.code.size(); ++i) {
            auto op = static_cast<OpCode>(fnChunk.code[i]);
            if (op == OpCode::GET_LOCAL) {
                hasGetLocal = true;
                ++i;  // advance past slot operand byte
            } else if (op == OpCode::GET_GLOBAL) {
                // A GET_GLOBAL whose name is "n" means the param wasn't allocated locally.
                ++i;  // advance i from opcode to name-index operand byte
                uint8_t nameIdx = fnChunk.code[i];  // read the operand
                if (nameIdx < fnChunk.names.size() && fnChunk.names[nameIdx] == "n") {
                    hasGetGlobalForParam = true;
                }
            } else if (op == OpCode::CONSTANT || op == OpCode::SET_GLOBAL || op == OpCode::SET_LOCAL ||
                       op == OpCode::CALL || op == OpCode::GET_PROPERTY || op == OpCode::SET_PROPERTY ||
                       op == OpCode::LOAD_MODULE) {
                ++i;  // skip single operand byte
            } else if (op == OpCode::JUMP || op == OpCode::JUMP_IF_FALSE || op == OpCode::LOOP) {
                i += 2;  // skip two-byte operand
            }
        }
        REQUIRE(hasGetLocal);
        REQUIRE_FALSE(hasGetGlobalForParam);
    }
}

TEST_CASE("Register allocation: function calls produce correct results", "[optimizer][register-alloc]") {
    SECTION("Single parameter function") {
        // Use the VM with --vm bytecode path to verify end-to-end correctness.
        auto program = parseCode(R"(
            fn negate(x) {
                return x * -1;
            }
        )");
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        REQUIRE_NOTHROW(vm.run(chunk));

        // Call negate(7) by running a second chunk that invokes it.
        auto callProgram = parseCode("var r = negate(7);");
        BytecodeCompiler callCompiler;
        Chunk callChunk = callCompiler.compile(callProgram);

        // The function 'negate' is now in the VM's global scope from the
        // first run; run the call chunk in the same VM.
        REQUIRE_NOTHROW(vm.run(callChunk));
        const auto& globals = vm.getGlobals();
        auto it = globals.find("r");
        REQUIRE(it != globals.end());
        REQUIRE(std::holds_alternative<double>(it->second));
        REQUIRE(std::get<double>(it->second) == -7.0);
    }

    SECTION("Multi-parameter function") {
        auto program = parseCode(R"(
            fn add(a, b) {
                return a + b;
            }
        )");
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        REQUIRE_NOTHROW(vm.run(chunk));

        auto callProgram = parseCode("var r = add(3, 4);");
        BytecodeCompiler callCompiler;
        Chunk callChunk = callCompiler.compile(callProgram);

        REQUIRE_NOTHROW(vm.run(callChunk));
        const auto& globals = vm.getGlobals();
        auto it = globals.find("r");
        REQUIRE(it != globals.end());
        REQUIRE(std::holds_alternative<double>(it->second));
        REQUIRE(std::get<double>(it->second) == 7.0);
    }

    SECTION("Recursive function") {
        auto program = parseCode(R"(
            fn factorial(n) {
                if (n <= 1) {
                    return 1;
                }
                return n * factorial(n - 1);
            }
        )");
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        REQUIRE_NOTHROW(vm.run(chunk));

        auto callProgram = parseCode("var r = factorial(5);");
        BytecodeCompiler callCompiler;
        Chunk callChunk = callCompiler.compile(callProgram);

        REQUIRE_NOTHROW(vm.run(callChunk));
        const auto& globals = vm.getGlobals();
        auto it = globals.find("r");
        REQUIRE(it != globals.end());
        REQUIRE(std::holds_alternative<double>(it->second));
        REQUIRE(std::get<double>(it->second) == 120.0);
    }
}

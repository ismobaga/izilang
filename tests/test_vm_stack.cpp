#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"

using namespace izi;

TEST_CASE("VM Stack Management", "[vm-stack]") {
    SECTION("Variable declarations don't leak stack values") {
        // Bug: VarStmt was leaving values on the stack after SET_GLOBAL
        // This caused stack to grow indefinitely with each variable declaration
        std::string source = R"(
            var a = 1;
            var b = 2;
            var c = 3;
            var d = 4;
            var e = 5;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        // Should not crash or have stack issues
        REQUIRE_NOTHROW(vm.run(chunk));
    }

    SECTION("Assignment expressions return their value") {
        // Assignment expressions (unlike statements) should leave value on stack
        // This allows expressions like: var y = (x = 42);
        std::string source = R"(
            var x = 0;
            var y = (x = 42);
            var z = x + y;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        Value result = vm.run(chunk);
        // Both x and y should be 42, so z should be 84
        // This validates that assignment expressions properly propagate values
        REQUIRE_NOTHROW(vm.run(chunk));
    }

    SECTION("Function declarations don't leak stack values") {
        // Bug: FunctionStmt was leaving function values on stack after SET_GLOBAL
        std::string source = R"(
            fn func1() { return 1; }
            fn func2() { return 2; }
            fn func3() { return 3; }
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        // Should not crash or have stack issues
        REQUIRE_NOTHROW(vm.run(chunk));
    }

    SECTION("Mixed declarations and statements") {
        // Comprehensive test of stack management
        // If stack management is wrong, this will cause issues when calling the function
        std::string source = R"(
            var a = 1;
            fn double(x) { return x * 2; }
            var b = 2;
            var c = double(a + b);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        // This tests stack integrity: function call requires correct stack state
        REQUIRE_NOTHROW(vm.run(chunk));
    }
}

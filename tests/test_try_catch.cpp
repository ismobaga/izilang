#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"

using namespace izi;

TEST_CASE("VM: Try-Catch basic exception handling", "[vm][exception]") {
    SECTION("Basic try-catch with thrown exception") {
        // Test that exception is caught and execution continues
        std::string source =
            "var caught = 0; try { throw \"test error\"; caught = 999; } catch(e) { caught = 1; } var after = 2;";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        REQUIRE_NOTHROW(vm.run(chunk));

        const auto& globals = vm.getGlobals();
        auto caughtIt = globals.find("caught");
        auto afterIt = globals.find("after");

        REQUIRE(caughtIt != globals.end());
        REQUIRE(afterIt != globals.end());
        REQUIRE(std::holds_alternative<double>(caughtIt->second));
        REQUIRE(std::holds_alternative<double>(afterIt->second));
        REQUIRE(std::get<double>(caughtIt->second) == 1.0);
        REQUIRE(std::get<double>(afterIt->second) == 2.0);
    }

    SECTION("Try-catch with no exception thrown") {
        // Test that catch block is skipped when no exception
        std::string source = "var executed = 0; try { executed = 1; } catch(e) { executed = 999; }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        REQUIRE_NOTHROW(vm.run(chunk));

        const auto& globals = vm.getGlobals();
        auto it = globals.find("executed");
        REQUIRE(it != globals.end());
        REQUIRE(std::holds_alternative<double>(it->second));
        REQUIRE(std::get<double>(it->second) == 1.0);
    }
}

TEST_CASE("VM: Try-Finally blocks", "[vm][exception]") {
    SECTION("Finally block executes without exception") {
        // Test that finally executes even when no exception
        std::string source = "var state = 0; try { state = 1; } finally { state = state + 10; }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        REQUIRE_NOTHROW(vm.run(chunk));

        const auto& globals = vm.getGlobals();
        auto it = globals.find("state");
        REQUIRE(it != globals.end());
        REQUIRE(std::holds_alternative<double>(it->second));
        REQUIRE(std::get<double>(it->second) == 11.0);
    }

    SECTION("Finally block executes with exception") {
        // Test that finally executes after catch
        std::string source =
            "var state = 0; try { throw \"error\"; } catch(e) { state = 1; } finally { state = state + 10; }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        REQUIRE_NOTHROW(vm.run(chunk));

        const auto& globals = vm.getGlobals();
        auto it = globals.find("state");
        REQUIRE(it != globals.end());
        REQUIRE(std::holds_alternative<double>(it->second));
        REQUIRE(std::get<double>(it->second) == 11.0);
    }

    SECTION("Try-finally without catch propagates after finally") {
        std::string source = "var state = 0; try { throw \"boom\"; } finally { state = 1; } var after = 2;";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        REQUIRE_NOTHROW(vm.run(chunk));

        const auto& globals = vm.getGlobals();
        auto stateIt = globals.find("state");
        auto afterIt = globals.find("after");

        REQUIRE(stateIt != globals.end());
        REQUIRE(std::holds_alternative<double>(stateIt->second));
        REQUIRE(std::get<double>(stateIt->second) == 1.0);
        REQUIRE(afterIt == globals.end());
    }
}

TEST_CASE("VM: Nested try-catch blocks", "[vm][exception]") {
    SECTION("Inner exception caught by inner catch") {
        // Test that nested exception handlers work correctly
        std::string source =
            "var state = 0; try { state = 1; try { throw \"inner\"; } catch(e) { state = 2; } state = 3; } catch(e) { "
            "state = 999; }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        REQUIRE_NOTHROW(vm.run(chunk));

        const auto& globals = vm.getGlobals();
        auto it = globals.find("state");
        REQUIRE(it != globals.end());
        REQUIRE(std::holds_alternative<double>(it->second));
        REQUIRE(std::get<double>(it->second) == 3.0);

        // Inner catch variable binding should not leak to outer/global scope.
        REQUIRE(globals.find("e") == globals.end());
    }

    SECTION("Catch variable restores previous global binding") {
        std::string source =
            "var e = \"outer\"; var seen = \"\"; try { throw \"inner\"; } catch(e) { seen = e; } var after = e;";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        REQUIRE_NOTHROW(vm.run(chunk));

        const auto& globals = vm.getGlobals();
        auto seenIt = globals.find("seen");
        auto afterIt = globals.find("after");

        REQUIRE(seenIt != globals.end());
        REQUIRE(afterIt != globals.end());
        REQUIRE(std::holds_alternative<std::string>(seenIt->second));
        REQUIRE(std::holds_alternative<std::string>(afterIt->second));
        REQUIRE(std::get<std::string>(seenIt->second) == "inner");
        REQUIRE(std::get<std::string>(afterIt->second) == "outer");
    }

}

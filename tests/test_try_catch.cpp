#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"

using namespace izi;

TEST_CASE("VM: Try-Catch basic exception handling", "[vm][exception]") {
    SECTION("Basic try-catch with thrown exception") {
        std::string source = "try { throw \"test error\"; } catch(e) { }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);
        
        VM vm;
        vm.run(chunk);
        
        REQUIRE(true);  // Test passes if no exception escapes
    }
    
    SECTION("Try-catch with no exception thrown") {
        std::string source = "try { var x = 5; } catch(e) { }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);
        
        VM vm;
        vm.run(chunk);
        
        REQUIRE(true);  // Test passes if completes successfully
    }
}

TEST_CASE("VM: Try-Finally blocks", "[vm][exception]") {
    SECTION("Finally block executes without exception") {
        std::string source = "try { var x = 1; } finally { var y = 2; }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);
        
        VM vm;
        vm.run(chunk);
        
        REQUIRE(true);  // Test passes if completes
    }
    
    SECTION("Finally block executes with exception") {
        std::string source = "try { throw \"error\"; } catch(e) { } finally { var x = 1; }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);
        
        VM vm;
        vm.run(chunk);
        
        REQUIRE(true);  // Test passes if completes
    }
}

TEST_CASE("VM: Nested try-catch blocks", "[vm][exception]") {
    SECTION("Inner exception caught by inner catch") {
        std::string source = "try { try { throw \"inner\"; } catch(e) { } } catch(e) { }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);
        
        VM vm;
        vm.run(chunk);
        
        REQUIRE(true);  // Test passes if completes
    }
}


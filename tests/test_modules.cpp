#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

TEST_CASE("Native module system - math module", "[modules][math]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "math";
            var result = math.pi;
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
    
    SECTION("Named imports work") {
        std::string source = R"(
            import { sqrt, pi } from "math";
            var result = sqrt(16);
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
    
    SECTION("Wildcard import works") {
        std::string source = R"(
            import * as m from "math";
            var result = m.sqrt(16);
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
    
    SECTION("Math functions work through module") {
        std::string source = R"(
            import "math";
            var pi_val = math.pi;
            var sqrt_val = math.sqrt(16);
            var pow_val = math.pow(2, 3);
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - string module", "[modules][string]") {
    SECTION("String module functions work") {
        std::string source = R"(
            import "string";
            var upper = string.toUpper("hello");
            var parts = string.split("a,b,c", ",");
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - array module", "[modules][array]") {
    SECTION("Array module functions work") {
        std::string source = R"(
            import { map, filter } from "array";
            var nums = [1, 2, 3, 4, 5];
            fn double(x) { return x * 2; }
            var doubled = map(nums, double);
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - io module", "[modules][io]") {
    SECTION("IO module functions work") {
        std::string source = R"(
            import "io";
            io.writeFile("/tmp/test_modules.txt", "test");
            var exists = io.exists("/tmp/test_modules.txt");
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - module deduplication", "[modules]") {
    SECTION("Importing same module twice works") {
        std::string source = R"(
            import "math";
            import { sqrt } from "math";
            var result1 = math.sqrt(16);
            var result2 = sqrt(25);
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - backward compatibility", "[modules]") {
    SECTION("Global functions still work") {
        std::string source = R"(
            var result = sqrt(16);
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

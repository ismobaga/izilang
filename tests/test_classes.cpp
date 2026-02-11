#include "catch.hpp"
#include "common/value.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>
#include <iostream>

using namespace izi;

// Helper to capture stdout
class OutputCapture {
public:
    OutputCapture() : old_buf(std::cout.rdbuf()) {
        std::cout.rdbuf(buffer.rdbuf());
    }
    ~OutputCapture() {
        std::cout.rdbuf(old_buf);
    }
    std::string getOutput() {
        return buffer.str();
    }
private:
    std::stringstream buffer;
    std::streambuf* old_buf;
};

TEST_CASE("Classes: Basic instantiation", "[classes]") {
    SECTION("Empty class instantiation") {
        std::string code = R"(
            class Empty { }
            var e = Empty();
            print(e);
        )";
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        Interpreter interp;
        OutputCapture capture;
        
        REQUIRE_NOTHROW(interp.interpret(stmts));
        
        std::string output = capture.getOutput();
        REQUIRE(output.find("<Empty instance>") != std::string::npos);
    }
    
    SECTION("Class with fields") {
        std::string code = R"(
            class Point {
                var x: Number;
                var y: Number;
            }
            var p = Point();
            print(p.x);
            print(p.y);
        )";
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        Interpreter interp;
        OutputCapture capture;
        
        REQUIRE_NOTHROW(interp.interpret(stmts));
        
        std::string output = capture.getOutput();
        REQUIRE(output.find("nil") != std::string::npos);
    }
}

TEST_CASE("Classes: Constructor", "[classes]") {
    SECTION("Constructor with parameters") {
        std::string code = R"(
            class Point {
                var x: Number;
                var y: Number;
                
                fn constructor(x: Number, y: Number) {
                    this.x = x;
                    this.y = y;
                }
            }
            var p = Point(10, 20);
            print(p.x);
            print(p.y);
        )";
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        Interpreter interp;
        OutputCapture capture;
        
        REQUIRE_NOTHROW(interp.interpret(stmts));
        
        std::string output = capture.getOutput();
        REQUIRE(output.find("10") != std::string::npos);
        REQUIRE(output.find("20") != std::string::npos);
    }
}

TEST_CASE("Classes: Property access", "[classes]") {
    SECTION("Get and set properties") {
        std::string code = R"(
            class Counter {
                var count: Number;
            }
            var c = Counter();
            c.count = 5;
            print(c.count);
            c.count = c.count + 1;
            print(c.count);
        )";
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        Interpreter interp;
        OutputCapture capture;
        
        REQUIRE_NOTHROW(interp.interpret(stmts));
        
        std::string output = capture.getOutput();
        REQUIRE(output.find("5") != std::string::npos);
        REQUIRE(output.find("6") != std::string::npos);
    }
}

TEST_CASE("Classes: Methods", "[classes]") {
    SECTION("Method call with this") {
        std::string code = R"(
            class Calculator {
                var value: Number;
                
                fn constructor(v: Number) {
                    this.value = v;
                }
                
                fn add(n: Number): Number {
                    this.value = this.value + n;
                    return this.value;
                }
                
                fn getValue(): Number {
                    return this.value;
                }
            }
            
            var calc = Calculator(10);
            print(calc.getValue());
            var result = calc.add(5);
            print(result);
            print(calc.getValue());
        )";
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        Interpreter interp;
        OutputCapture capture;
        
        REQUIRE_NOTHROW(interp.interpret(stmts));
        
        std::string output = capture.getOutput();
        // Should print: 10, 15, 15
        size_t first_10 = output.find("10");
        REQUIRE(first_10 != std::string::npos);
        size_t first_15 = output.find("15", first_10);
        REQUIRE(first_15 != std::string::npos);
        size_t second_15 = output.find("15", first_15 + 2);
        REQUIRE(second_15 != std::string::npos);
    }
}

TEST_CASE("Classes: Error handling", "[classes]") {
    SECTION("Undefined property access") {
        std::string code = R"(
            class Empty { }
            var e = Empty();
            print(e.nonexistent);
        )";
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        Interpreter interp;
        
        REQUIRE_THROWS_AS(interp.interpret(stmts), RuntimeError);
    }
    
    SECTION("This outside of method") {
        std::string code = R"(
            print(this);
        )";
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        Interpreter interp;
        
        REQUIRE_THROWS_AS(interp.interpret(stmts), RuntimeError);
    }
}

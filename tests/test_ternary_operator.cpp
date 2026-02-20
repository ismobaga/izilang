#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "interp/interpreter.hpp"
#include "ast/expr.hpp"
#include <sstream>

using namespace izi;

// Helper function to capture stdout
class OutputCapture {
public:
    OutputCapture() {
        oldBuf = std::cout.rdbuf();
        std::cout.rdbuf(buffer.rdbuf());
    }
    
    ~OutputCapture() {
        std::cout.rdbuf(oldBuf);
    }
    
    std::string getOutput() {
        return buffer.str();
    }
    
private:
    std::stringstream buffer;
    std::streambuf* oldBuf;
};

TEST_CASE("Ternary operator: Basic syntax", "[ternary]") {
    SECTION("Simple true condition") {
        std::string source = "print(true ? 1 : 2);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n");
    }
    
    SECTION("Simple false condition") {
        std::string source = "print(false ? 1 : 2);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "2\n");
    }
    
    SECTION("Comparison in condition") {
        std::string source = "print(5 > 3 ? \"yes\" : \"no\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "yes\n");
    }
    
    SECTION("Comparison with less than") {
        std::string source = "print(2 < 1 ? \"less\" : \"greater\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "greater\n");
    }
}

TEST_CASE("Ternary operator: With variables", "[ternary]") {
    SECTION("Variable in condition") {
        std::string source = R"(
            var x = 10;
            print(x > 5 ? "big" : "small");
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "big\n");
    }
    
    SECTION("Variables in branches") {
        std::string source = R"(
            var a = 100;
            var b = 200;
            var result = true ? a : b;
            print(result);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "100\n");
    }
    
    SECTION("Assignment with ternary") {
        std::string source = R"(
            var score = 85;
            var grade = score >= 90 ? "A" : score >= 80 ? "B" : "C";
            print(grade);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "B\n");
    }
}

TEST_CASE("Ternary operator: Nested ternary", "[ternary]") {
    SECTION("Nested in then branch") {
        std::string source = "print(true ? (true ? 1 : 2) : 3);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n");
    }
    
    SECTION("Nested in else branch") {
        std::string source = "print(false ? 1 : (true ? 2 : 3));";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "2\n");
    }
    
    SECTION("Right-associative chaining") {
        std::string source = R"(
            var x = 15;
            print(x < 10 ? "small" : x < 20 ? "medium" : "large");
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "medium\n");
    }
}

TEST_CASE("Ternary operator: Complex expressions", "[ternary]") {
    SECTION("Arithmetic in branches") {
        std::string source = "print(true ? 2 + 3 : 4 + 5);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "5\n");
    }
    
    SECTION("Complex condition") {
        std::string source = "print((5 > 3 and 10 < 20) ? \"both true\" : \"not both\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "both true\n");
    }
    
    SECTION("Ternary with function call") {
        std::string source = R"(
            fn max(a, b) {
                return a > b ? a : b;
            }
            print(max(10, 20));
            print(max(30, 15));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "20\n30\n");
    }
}

TEST_CASE("Ternary operator: Type consistency", "[ternary]") {
    SECTION("Different types in branches") {
        std::string source = "print(true ? 42 : \"string\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "42\n");
    }
    
    SECTION("Nil in branches") {
        std::string source = "print(false ? nil : \"not nil\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "not nil\n");
    }
}

TEST_CASE("Ternary operator: Truthiness", "[ternary]") {
    SECTION("Truthy numbers") {
        std::string source = "print(5 ? \"truthy\" : \"falsy\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "truthy\n");
    }
    
    SECTION("Falsy zero") {
        std::string source = "print(0 ? \"truthy\" : \"falsy\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "falsy\n");
    }
    
    SECTION("Falsy nil") {
        std::string source = "print(nil ? \"truthy\" : \"falsy\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "falsy\n");
    }
    
    SECTION("Truthy string") {
        std::string source = "print(\"hello\" ? \"truthy\" : \"falsy\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "truthy\n");
    }
}

TEST_CASE("Ternary operator: Precedence", "[ternary]") {
    SECTION("Ternary with assignment") {
        std::string source = R"(
            var x = 5 > 3 ? 10 : 20;
            print(x);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "10\n");
    }
    
    SECTION("Ternary with logical or") {
        std::string source = "print((false or true) ? \"yes\" : \"no\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "yes\n");
    }
}

TEST_CASE("Ternary operator: Short-circuit evaluation", "[ternary]") {
    SECTION("Only evaluate then branch when true") {
        std::string source = R"(
            var executed = 0;
            fn side_effect() {
                executed = executed + 1;
                return executed;
            }
            var result = true ? 1 : side_effect();
            print(executed);
            print(result);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "0\n1\n");
    }
    
    SECTION("Only evaluate else branch when false") {
        std::string source = R"(
            var executed = 0;
            fn side_effect() {
                executed = executed + 1;
                return executed;
            }
            var result = false ? side_effect() : 2;
            print(executed);
            print(result);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "0\n2\n");
    }
}

TEST_CASE("Ternary operator: Parser structure", "[ternary][parser]") {
    SECTION("Parse creates ConditionalExpr") {
        std::string source = "true ? 1 : 2;";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        REQUIRE(program.size() == 1);
        auto* exprStmt = dynamic_cast<ExprStmt*>(program[0].get());
        REQUIRE(exprStmt != nullptr);
        
        auto* condExpr = dynamic_cast<ConditionalExpr*>(exprStmt->expr.get());
        REQUIRE(condExpr != nullptr);
        REQUIRE(condExpr->condition != nullptr);
        REQUIRE(condExpr->thenBranch != nullptr);
        REQUIRE(condExpr->elseBranch != nullptr);
    }
}

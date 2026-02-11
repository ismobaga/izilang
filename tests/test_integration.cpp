#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "interp/interpreter.hpp"
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

TEST_CASE("Integration: Simple arithmetic expressions", "[integration]") {
    SECTION("Addition") {
        std::string source = "print(2 + 3);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "5\n");
    }
    
    SECTION("Subtraction") {
        std::string source = "print(10 - 4);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "6\n");
    }
    
    SECTION("Multiplication") {
        std::string source = "print(3 * 4);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "12\n");
    }
    
    SECTION("Division") {
        std::string source = "print(20 / 5);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "4\n");
    }
}

TEST_CASE("Integration: Variable declaration and usage", "[integration]") {
    SECTION("Declare and print variable") {
        std::string source = "var x = 42; print(x);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "42\n");
    }
    
    SECTION("Variable assignment") {
        std::string source = "var x = 10; x = 20; print(x);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "20\n");
    }
}

TEST_CASE("Integration: String operations", "[integration]") {
    SECTION("String concatenation") {
        std::string source = "print(\"hello\" + \" world\");";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "hello world\n");
    }
}

TEST_CASE("Integration: Boolean operations", "[integration]") {
    SECTION("Equality") {
        std::string source = "print(5 == 5);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("Inequality") {
        std::string source = "print(5 != 3);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("Less than") {
        std::string source = "print(3 < 5);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("Greater than") {
        std::string source = "print(5 > 3);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "true\n");
    }
}

TEST_CASE("Integration: If statements", "[integration]") {
    SECTION("If true branch") {
        std::string source = "if (true) { print(\"yes\"); }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "yes\n");
    }
    
    SECTION("If false branch with else") {
        std::string source = "if (false) { print(\"yes\"); } else { print(\"no\"); }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "no\n");
    }
}

TEST_CASE("Integration: While loops", "[integration]") {
    SECTION("Simple while loop") {
        std::string source = "var i = 0; while (i < 3) { print(i); i = i + 1; }";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "0\n1\n2\n");
    }
}

TEST_CASE("Integration: Functions", "[integration]") {
    SECTION("Simple function call") {
        std::string source = R"(
            fn add(a, b) {
                return a + b;
            }
            print(add(3, 4));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "7\n");
    }
    
    SECTION("Recursive function") {
        std::string source = R"(
            fn factorial(n) {
                if (n <= 1) {
                    return 1;
                }
                return n * factorial(n - 1);
            }
            print(factorial(5));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "120\n");
    }
}

TEST_CASE("Integration: Arrays", "[integration]") {
    SECTION("Array creation and access") {
        std::string source = "var arr = [1, 2, 3]; print(arr[0]); print(arr[1]); print(arr[2]);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n2\n3\n");
    }
    
    SECTION("Array assignment") {
        std::string source = "var arr = [1, 2, 3]; arr[1] = 10; print(arr[1]);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "10\n");
    }
}

TEST_CASE("Integration: Maps", "[integration]") {
    SECTION("Map creation and access") {
        std::string source = R"(var map = {"key": 42}; print(map["key"]);)";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "42\n");
    }
    
    SECTION("Map assignment") {
        std::string source = R"(var map = {"key": 42}; map["key"] = 100; print(map["key"]);)";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "100\n");
    }
}

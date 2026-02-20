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
    
    SECTION("Modulo") {
        std::string source = "print(10 % 3);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n");
    }
    
    SECTION("Modulo zero remainder") {
        std::string source = "print(9 % 3);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "0\n");
    }
    
    SECTION("Modulo by zero throws error") {
        std::string source = "print(10 % 0);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_THROWS_AS(interp.interpret(program), RuntimeError);
    }
}

TEST_CASE("Integration: Compound assignment operators", "[integration]") {
    SECTION("Plus-equal") {
        std::string source = "var x = 5; x += 3; print(x);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "8\n");
    }
    
    SECTION("Minus-equal") {
        std::string source = "var x = 10; x -= 4; print(x);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "6\n");
    }
    
    SECTION("Star-equal") {
        std::string source = "var x = 3; x *= 4; print(x);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "12\n");
    }
    
    SECTION("Slash-equal") {
        std::string source = "var x = 20; x /= 5; print(x);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "4\n");
    }
    
    SECTION("Percent-equal") {
        std::string source = "var x = 10; x %= 3; print(x);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n");
    }
    
    SECTION("String concatenation with plus-equal") {
        std::string source = "var s = \"hello\"; s += \" world\"; print(s);";
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

TEST_CASE("Integration: Short-circuit evaluation", "[integration]") {
    SECTION("AND operator - false left operand") {
        std::string source = "print(false and true);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "false\n");
    }
    
    SECTION("AND operator - true left operand") {
        std::string source = "print(true and false);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "false\n");
    }
    
    SECTION("AND operator - both true") {
        std::string source = "print(true and true);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("OR operator - true left operand") {
        std::string source = "print(true or false);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("OR operator - false left operand") {
        std::string source = "print(false or true);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("OR operator - both false") {
        std::string source = "print(false or false);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "false\n");
    }
    
    SECTION("AND short-circuit - right operand not evaluated") {
        // This test verifies that the right operand is not evaluated when left is false
        // If evaluated, it would cause a division by zero error
        std::string source = "var x = 0; print(false and (10 / x > 5));";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        // Should not throw an error due to short-circuit
        REQUIRE_NOTHROW(interp.interpret(program));
        REQUIRE(capture.getOutput() == "false\n");
    }
    
    SECTION("OR short-circuit - right operand not evaluated") {
        // This test verifies that the right operand is not evaluated when left is true
        // If evaluated, it would cause a division by zero error
        std::string source = "var x = 0; print(true or (10 / x > 5));";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        // Should not throw an error due to short-circuit
        REQUIRE_NOTHROW(interp.interpret(program));
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("Complex expression with AND and OR") {
        std::string source = "print(true or false and false);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        // AND has higher precedence: false and false = false, then true or false = true
        REQUIRE(capture.getOutput() == "true\n");
    }
    
    SECTION("AND returns last evaluated value") {
        std::string source = "print(5 and 10);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        // Both are truthy, should return the right value
        REQUIRE(capture.getOutput() == "10\n");
    }
    
    SECTION("OR returns first truthy value") {
        std::string source = "print(5 or 10);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        // First value is truthy, should return it
        REQUIRE(capture.getOutput() == "5\n");
    }
    
    SECTION("AND short-circuit - side effect not executed") {
        // Verify right operand is not evaluated by checking variable is unchanged
        std::string source = "var counter = 0; var result = false and (counter = 1); print(counter);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        // counter should still be 0 because right side was not evaluated
        REQUIRE(capture.getOutput() == "0\n");
    }
    
    SECTION("OR short-circuit - side effect not executed") {
        // Verify right operand is not evaluated by checking variable is unchanged
        std::string source = "var counter = 0; var result = true or (counter = 1); print(counter);";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        // counter should still be 0 because right side was not evaluated
        REQUIRE(capture.getOutput() == "0\n");
    }
}

TEST_CASE("Exception handling: Basic try-catch", "[integration][exceptions]") {
    SECTION("Catch simple string exception") {
        std::string source = R"(
            try {
                throw "error message";
            } catch (e) {
                print(e);
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "error message\n");
    }
    
    SECTION("Try block without exception") {
        std::string source = R"(
            var executed = false;
            try {
                executed = true;
                print("in try");
            } catch (e) {
                print("in catch");
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "in try\n");
    }
    
    SECTION("Exception propagates if not caught") {
        std::string source = R"(
            throw "uncaught error";
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        Interpreter interp(source);
        REQUIRE_THROWS_AS(interp.interpret(program), ThrowSignal);
    }
}

TEST_CASE("Exception handling: Try-catch-finally", "[integration][exceptions]") {
    SECTION("Finally always executes with no exception") {
        std::string source = R"(
            try {
                print("try");
            } finally {
                print("finally");
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "try\nfinally\n");
    }
    
    SECTION("Finally always executes with exception") {
        std::string source = R"(
            try {
                print("try");
                throw "error";
            } catch (e) {
                print("catch");
            } finally {
                print("finally");
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "try\ncatch\nfinally\n");
    }
    
    SECTION("Finally executes even if exception not caught") {
        std::string source = R"(
            try {
                throw "error";
            } finally {
                print("cleanup");
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        REQUIRE_THROWS_AS(interp.interpret(program), ThrowSignal);
        
        // Finally block should have executed before exception propagated
        REQUIRE(capture.getOutput() == "cleanup\n");
    }
}

TEST_CASE("Exception handling: Nested try-catch", "[integration][exceptions]") {
    SECTION("Inner catch handles exception") {
        std::string source = R"(
            try {
                print("outer try");
                try {
                    print("inner try");
                    throw "inner error";
                } catch (e) {
                    print("inner catch");
                }
                print("after inner");
            } catch (e) {
                print("outer catch");
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "outer try\ninner try\ninner catch\nafter inner\n");
    }
    
    SECTION("Exception propagates to outer catch") {
        std::string source = R"(
            try {
                print("outer try");
                try {
                    print("inner try");
                    throw "error";
                } finally {
                    print("inner finally");
                }
            } catch (e) {
                print("outer catch");
                print(e);
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "outer try\ninner try\ninner finally\nouter catch\nerror\n");
    }
}

TEST_CASE("Exception handling: Function with try-catch", "[integration][exceptions]") {
    SECTION("Exception thrown in function") {
        std::string source = R"(
            fn riskyOperation() {
                throw "operation failed";
            }
            
            try {
                riskyOperation();
            } catch (e) {
                print("caught:");
                print(e);
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "caught:\noperation failed\n");
    }
    
    SECTION("Division by zero with custom error") {
        std::string source = R"(
            fn divide(a, b) {
                if (b == 0) {
                    throw "Division by zero";
                }
                return a / b;
            }
            
            try {
                var result = divide(10, 0);
                print(result);
            } catch (e) {
                print("Error:");
                print(e);
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Error:\nDivision by zero\n");
    }
}

TEST_CASE("Exception handling: Different value types", "[integration][exceptions]") {
    SECTION("Throw number") {
        std::string source = R"(
            try {
                throw 42;
            } catch (e) {
                print(e);
            }
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "42\n");
    }
    
    SECTION("Throw boolean") {
        std::string source = R"(
            try {
                throw true;
            } catch (e) {
                print(e);
            }
        )";
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

TEST_CASE("Integration: String interpolation", "[integration]") {
    SECTION("Simple variable interpolation") {
        std::string source = R"(
            var name = "Alice";
            var message = "Hello, ${name}!";
            print(message);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Hello, Alice!\n");
    }
    
    SECTION("Number interpolation") {
        std::string source = R"(
            var age = 30;
            print("Age: ${age}");
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Age: 30\n");
    }
    
    SECTION("Multiple interpolations") {
        std::string source = R"(
            var name = "Bob";
            var age = 25;
            print("Name: ${name}, Age: ${age}");
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Name: Bob, Age: 25\n");
    }
    
    SECTION("Expression in interpolation") {
        std::string source = R"(
            var x = 5;
            var y = 10;
            print("Sum: ${x + y}");
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Sum: 15\n");
    }
    
    SECTION("Boolean interpolation") {
        std::string source = R"(
            var flag = true;
            print("Flag: ${flag}");
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Flag: true\n");
    }
    
    SECTION("Empty interpolation at start") {
        std::string source = R"(
            var x = 42;
            print("${x}");
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "42\n");
    }
}

TEST_CASE("Integration: v0.3 - Types and Classes Combined", "[integration][v03]") {
    SECTION("Typed class with methods") {
        std::string source = R"(
            class Calculator {
                var result: Number;
                
                fn constructor(initial: Number) {
                    this.result = initial;
                }
                
                fn add(n: Number): Number {
                    this.result = this.result + n;
                    return this.result;
                }
                
                fn subtract(n: Number): Number {
                    this.result = this.result - n;
                    return this.result;
                }
                
                fn getResult(): Number {
                    return this.result;
                }
            }
            
            var calc = Calculator(10);
            print(calc.getResult());
            calc.add(5);
            print(calc.getResult());
            calc.subtract(3);
            print(calc.getResult());
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        std::string output = capture.getOutput();
        REQUIRE(output.find("10") != std::string::npos);
        REQUIRE(output.find("15") != std::string::npos);
        REQUIRE(output.find("12") != std::string::npos);
    }
    
    SECTION("Multiple typed classes interacting") {
        std::string source = R"(
            class Point {
                var x: Number;
                var y: Number;
                
                fn constructor(x: Number, y: Number) {
                    this.x = x;
                    this.y = y;
                }
                
                fn distanceFrom(other): Number {
                    var dx = this.x - other.x;
                    var dy = this.y - other.y;
                    return dx * dx + dy * dy;
                }
            }
            
            var p1 = Point(0, 0);
            var p2 = Point(3, 4);
            print(p1.distanceFrom(p2));
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        std::string output = capture.getOutput();
        REQUIRE(output.find("25") != std::string::npos); // 3^2 + 4^2 = 25
    }
    
    SECTION("Class with typed array field") {
        std::string source = R"(
            class Container {
                var items: Array<Number>;
                var count: Number;
                
                fn constructor() {
                    this.items = [1, 2, 3];
                    this.count = 3;
                }
                
                fn size(): Number {
                    return this.count;
                }
                
                fn getItem(index: Number): Number {
                    return this.items[index];
                }
            }
            
            var container = Container();
            print(container.size());
            print(container.getItem(0));
            print(container.getItem(2));
        )";
        
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        std::string output = capture.getOutput();
        REQUIRE(output.find("3") != std::string::npos);
        REQUIRE(output.find("1") != std::string::npos);
    }
}

TEST_CASE("Integration: Array spread operator", "[integration][spread]") {
    SECTION("Spread two arrays") {
        std::string source = R"(
            var arr1 = [1, 2, 3];
            var arr2 = [4, 5, 6];
            var combined = [...arr1, ...arr2];
            print(combined[0]);
            print(combined[5]);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n6\n");
    }
    
    SECTION("Spread with mixed elements") {
        std::string source = R"(
            var arr = [2, 3];
            var mixed = [1, ...arr, 4];
            print(mixed[0]);
            print(mixed[1]);
            print(mixed[2]);
            print(mixed[3]);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n2\n3\n4\n");
    }
}

TEST_CASE("Integration: Map spread operator", "[integration][spread]") {
    SECTION("Spread two maps") {
        std::string source = R"(
            var obj1 = {"name": "John", "age": 30};
            var obj2 = {"city": "NYC"};
            var merged = {...obj1, ...obj2};
            print(merged["name"]);
            print(merged["city"]);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "John\nNYC\n");
    }
    
    SECTION("Spread with override") {
        std::string source = R"(
            var obj1 = {"x": 1, "y": 2};
            var obj2 = {"y": 3, "z": 4};
            var merged = {...obj1, ...obj2};
            print(merged["x"]);
            print(merged["y"]);
            print(merged["z"]);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n3\n4\n");
    }
}

TEST_CASE("Integration: Array destructuring", "[integration][destructuring]") {
    SECTION("Basic array destructuring") {
        std::string source = R"(
            var arr = [1, 2, 3];
            var [a, b, c] = arr;
            print(a);
            print(b);
            print(c);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n2\n3\n");
    }
    
    SECTION("Destructuring with fewer variables") {
        std::string source = R"(
            var [x, y] = [10, 20, 30];
            print(x);
            print(y);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "10\n20\n");
    }
    
    SECTION("Destructuring with more variables (nil for missing)") {
        std::string source = R"(
            var [a, b, c] = [1, 2];
            print(a);
            print(b);
            print(c);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n2\nnil\n");
    }
}

TEST_CASE("Integration: Map destructuring", "[integration][destructuring]") {
    SECTION("Basic map destructuring") {
        std::string source = R"(
            var person = {"name": "Alice", "age": 25};
            var {name, age} = person;
            print(name);
            print(age);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Alice\n25\n");
    }
    
    SECTION("Destructuring with missing keys (nil)") {
        std::string source = R"(
            var data = {"x": 10};
            var {x, y, z} = data;
            print(x);
            print(y);
            print(z);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "10\nnil\nnil\n");
    }
}

TEST_CASE("Integration: Combined spread and destructuring", "[integration][spread][destructuring]") {
    SECTION("Destructure spread arrays") {
        std::string source = R"(
            var arr1 = [1, 2];
            var arr2 = [3, 4];
            var [a, b, c, d] = [...arr1, ...arr2];
            print(a);
            print(b);
            print(c);
            print(d);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "1\n2\n3\n4\n");
    }
    
    SECTION("Destructure spread maps") {
        std::string source = R"(
            var user = {"name": "Bob"};
            var profile = {"email": "bob@example.com"};
            var merged = {...user, ...profile};
            var {name, email} = merged;
            print(name);
            print(email);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        
        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);
        
        REQUIRE(capture.getOutput() == "Bob\nbob@example.com\n");
    }
}

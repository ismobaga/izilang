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

    ~OutputCapture() { std::cout.rdbuf(oldBuf); }

    std::string getOutput() { return buffer.str(); }

   private:
    std::stringstream buffer;
    std::streambuf* oldBuf;
};

TEST_CASE("First-class functions: Anonymous functions", "[first-class][anonymous]") {
    SECTION("Simple anonymous function assignment") {
        std::string source = R"(
            var add = fn(a, b) { return a + b; };
            print(add(2, 3));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "5\n");
    }

    SECTION("Anonymous function with no parameters") {
        std::string source = R"(
            var greet = fn() { return "Hello"; };
            print(greet());
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "Hello\n");
    }

    SECTION("Anonymous function with single parameter") {
        std::string source = R"(
            var square = fn(x) { return x * x; };
            print(square(5));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "25\n");
    }
}

TEST_CASE("First-class functions: Higher-order functions", "[first-class][higher-order]") {
    SECTION("Function as parameter") {
        std::string source = R"(
            fn apply(f, x) {
                return f(x);
            }
            
            fn double(n) {
                return n * 2;
            }
            
            print(apply(double, 5));
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

    SECTION("Higher-order function with anonymous function") {
        std::string source = R"(
            fn apply(f, x) {
                return f(x);
            }
            
            print(apply(fn(n) { return n * 2; }, 5));
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

    SECTION("Function returning result of another function") {
        std::string source = R"(
            fn compose(f, g, x) {
                return f(g(x));
            }
            
            fn addOne(n) { return n + 1; }
            fn double(n) { return n * 2; }
            
            print(compose(double, addOne, 5));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "12\n");
    }
}

TEST_CASE("First-class functions: Closures", "[first-class][closures]") {
    SECTION("Simple closure with counter") {
        std::string source = R"(
            fn makeCounter() {
                var count = 0;
                return fn() {
                    count = count + 1;
                    return count;
                };
            }
            
            var counter = makeCounter();
            print(counter());
            print(counter());
            print(counter());
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

    SECTION("Multiple independent closures") {
        std::string source = R"(
            fn makeCounter() {
                var count = 0;
                return fn() {
                    count = count + 1;
                    return count;
                };
            }
            
            var counter1 = makeCounter();
            var counter2 = makeCounter();
            
            print(counter1());
            print(counter1());
            print(counter2());
            print(counter1());
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "1\n2\n1\n3\n");
    }

    SECTION("Closure with parameter") {
        std::string source = R"(
            fn makeAdder(x) {
                return fn(y) {
                    return x + y;
                };
            }
            
            var add5 = makeAdder(5);
            var add10 = makeAdder(10);
            
            print(add5(3));
            print(add10(3));
            print(add5(7));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "8\n13\n12\n");
    }

    SECTION("Nested closures") {
        std::string source = R"(
            fn outer(a) {
                return fn(b) {
                    return fn(c) {
                        return a + b + c;
                    };
                };
            }
            
            var f = outer(1);
            var g = f(2);
            print(g(3));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "6\n");
    }
}

TEST_CASE("First-class functions: Complex scenarios", "[first-class][complex]") {
    SECTION("Function stored in variable and reassigned") {
        std::string source = R"(
            var f = fn(x) { return x * 2; };
            print(f(5));
            
            f = fn(x) { return x * 3; };
            print(f(5));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "10\n15\n");
    }

    SECTION("Closure modifying captured variable") {
        std::string source = R"(
            fn makeAccumulator() {
                var sum = 0;
                return fn(x) {
                    sum = sum + x;
                    return sum;
                };
            }
            
            var acc = makeAccumulator();
            print(acc(5));
            print(acc(3));
            print(acc(2));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "5\n8\n10\n");
    }

    SECTION("Returning different functions based on condition") {
        std::string source = R"(
            fn makeOperation(isAdd) {
                if (isAdd) {
                    return fn(a, b) { return a + b; };
                } else {
                    return fn(a, b) { return a - b; };
                }
            }
            
            var add = makeOperation(true);
            var sub = makeOperation(false);
            
            print(add(10, 5));
            print(sub(10, 5));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "15\n5\n");
    }
}

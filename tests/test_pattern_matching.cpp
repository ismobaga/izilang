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

TEST_CASE("Pattern Matching: Literal patterns", "[pattern_match]") {
    SECTION("Match number 0") {
        std::string source = R"(
            fn describe(value) {
                return match value {
                    0 => "zero",
                    1 => "one",
                    _ => "other"
                };
            }
            print(describe(0));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "zero\n");
    }

    SECTION("Match number 1") {
        std::string source = R"(
            fn describe(value) {
                return match value {
                    0 => "zero",
                    1 => "one",
                    _ => "other"
                };
            }
            print(describe(1));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "one\n");
    }

    SECTION("Match string literal") {
        std::string source = R"(
            fn greet(lang) {
                return match lang {
                    "en" => "Hello",
                    "fr" => "Bonjour",
                    "es" => "Hola",
                    _ => "Unknown"
                };
            }
            print(greet("fr"));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "Bonjour\n");
    }

    SECTION("Match boolean literals") {
        std::string source = R"(
            fn boolToStr(b) {
                return match b {
                    true => "yes",
                    false => "no",
                    _ => "other"
                };
            }
            print(boolToStr(true));
            print(boolToStr(false));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "yes\nno\n");
    }
}

TEST_CASE("Pattern Matching: Wildcard pattern", "[pattern_match]") {
    SECTION("Wildcard matches anything") {
        std::string source = R"(
            fn describe(value) {
                return match value {
                    0 => "zero",
                    1 => "one",
                    _ => "other"
                };
            }
            print(describe(42));
            print(describe(100));
            print(describe(-5));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "other\nother\nother\n");
    }
}

TEST_CASE("Pattern Matching: Variable patterns", "[pattern_match]") {
    SECTION("Variable pattern binds value") {
        std::string source = R"(
            fn double(value) {
                return match value {
                    x => x * 2
                };
            }
            print(double(5));
            print(double(10));
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

    SECTION("Variable pattern with multiple cases") {
        std::string source = R"(
            fn describe(value) {
                return match value {
                    0 => "zero",
                    x => x
                };
            }
            print(describe(0));
            print(describe(42));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "zero\n42\n");
    }
}

TEST_CASE("Pattern Matching: Guards", "[pattern_match]") {
    SECTION("Guard with variable pattern - positive numbers") {
        std::string source = R"(
            fn describe(value) {
                return match value {
                    0 => "zero",
                    1 => "one",
                    x if x > 0 => "positive",
                    x if x < 0 => "negative",
                    _ => "unknown"
                };
            }
            print(describe(5));
            print(describe(100));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "positive\npositive\n");
    }

    SECTION("Guard with variable pattern - negative numbers") {
        std::string source = R"(
            fn describe(value) {
                return match value {
                    0 => "zero",
                    1 => "one",
                    x if x > 0 => "positive",
                    x if x < 0 => "negative",
                    _ => "unknown"
                };
            }
            print(describe(-5));
            print(describe(-100));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "negative\nnegative\n");
    }

    SECTION("Guard with complex condition") {
        std::string source = R"(
            fn classify(n) {
                return match n {
                    x if x > 0 and x < 10 => "small positive",
                    x if x >= 10 and x < 100 => "medium positive",
                    x if x >= 100 => "large positive",
                    _ => "other"
                };
            }
            print(classify(5));
            print(classify(50));
            print(classify(500));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "small positive\nmedium positive\nlarge positive\n");
    }
}

TEST_CASE("Pattern Matching: Comprehensive example from issue", "[pattern_match]") {
    SECTION("Complete describe function") {
        std::string source = R"(
            fn describe(value) {
                return match value {
                    0 => "zero",
                    1 => "one",
                    x if x > 0 => "positive",
                    x if x < 0 => "negative",
                    _ => "unknown"
                };
            }
            print(describe(0));
            print(describe(1));
            print(describe(42));
            print(describe(-10));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "zero\none\npositive\nnegative\n");
    }
}

TEST_CASE("Pattern Matching: Edge cases", "[pattern_match]") {
    SECTION("Match expression as value") {
        std::string source = R"(
            var result = match 5 {
                0 => "zero",
                x => "non-zero"
            };
            print(result);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "non-zero\n");
    }

    SECTION("Nested match expressions") {
        std::string source = R"(
            fn classify(x, y) {
                return match x {
                    0 => match y {
                        0 => "both zero",
                        _ => "x is zero"
                    },
                    _ => "x not zero"
                };
            }
            print(classify(0, 0));
            print(classify(0, 5));
            print(classify(5, 0));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "both zero\nx is zero\nx not zero\n");
    }
}

TEST_CASE("Pattern Matching: First matching pattern wins", "[pattern_match]") {
    SECTION("Order matters") {
        std::string source = R"(
            fn test(n) {
                return match n {
                    x if x > 0 => "first",
                    5 => "second",
                    _ => "third"
                };
            }
            print(test(5));
            print(test(10));
            print(test(0));
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        OutputCapture capture;
        Interpreter interp(source);
        interp.interpret(program);

        REQUIRE(capture.getOutput() == "first\nfirst\nthird\n");
    }
}

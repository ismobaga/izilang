#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"

#include <sstream>
#include <stdexcept>

using namespace izi;

// Helper: capture stdout produced by interpreting source
static std::string runCode(const std::string& source) {
    std::ostringstream buf;
    std::streambuf* old = std::cout.rdbuf(buf.rdbuf());
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        Interpreter interp(source);
        interp.interpret(program);
    } catch (...) {
        std::cout.rdbuf(old);
        throw;
    }
    std::cout.rdbuf(old);
    return buf.str();
}

// Helper: just parse without running (throws on parse errors)
static void parseCode(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    parser.parse();
}

TEST_CASE("Macros: definition and expression expansion", "[macros]") {
    SECTION("Zero-argument expression macro") {
        std::string src = R"(
            macro pi() { 3.14159 }
            print(pi!());
        )";
        REQUIRE(runCode(src) == "3.14159\n");
    }

    SECTION("Single-argument expression macro") {
        std::string src = R"(
            macro double(x) { x * 2 }
            print(double!(5));
        )";
        REQUIRE(runCode(src) == "10\n");
    }

    SECTION("Two-argument expression macro") {
        std::string src = R"(
            macro add(a, b) { a + b }
            print(add!(3, 4));
        )";
        REQUIRE(runCode(src) == "7\n");
    }

    SECTION("max macro selects larger value") {
        std::string src = R"(
            macro max(a, b) { a > b ? a : b }
            print(max!(10, 20));
            print(max!(30, 5));
        )";
        REQUIRE(runCode(src) == "20\n30\n");
    }

    SECTION("min macro selects smaller value") {
        std::string src = R"(
            macro min(a, b) { a < b ? a : b }
            print(min!(3, 7));
        )";
        REQUIRE(runCode(src) == "3\n");
    }
}

TEST_CASE("Macros: statement expansion", "[macros]") {
    SECTION("Statement macro with print") {
        std::string src = R"(
            macro log(msg) { print(msg); }
            log!("hello macro");
        )";
        REQUIRE(runCode(src) == "hello macro\n");
    }

    SECTION("assert macro does not throw when condition is true") {
        std::string src = R"(
            macro assert(cond, msg) {
                if (!cond) { throw msg; }
            }
            var ok = true;
            assert!(ok, "should not throw");
        )";
        REQUIRE_NOTHROW(runCode(src));
    }

    SECTION("assert macro throws when condition is false") {
        std::string src = R"(
            macro assert(cond, msg) {
                if (!cond) { throw msg; }
            }
            assert!(false, "assertion failed");
        )";
        REQUIRE_THROWS(runCode(src));
    }

    SECTION("Swap macro exchanges two variables") {
        std::string src = R"(
            macro swap(a, b) {
                var _tmp = a;
                a = b;
                b = _tmp;
            }
            var x = 1;
            var y = 2;
            swap!(x, y);
            print(x);
            print(y);
        )";
        REQUIRE(runCode(src) == "2\n1\n");
    }
}

TEST_CASE("Macros: arguments with complex expressions", "[macros]") {
    SECTION("Macro argument containing arithmetic") {
        std::string src = R"(
            macro square(n) { n * n }
            print(square!(3 + 1));
        )";
        // square!(3 + 1) expands to 3 + 1 * 3 + 1 (token substitution without extra parens).
        // Operator precedence: 3 + (1 * 3) + 1 = 7.
        REQUIRE(runCode(src) == "7\n");
    }

    SECTION("Macro argument containing function call") {
        std::string src = R"(
            macro wrap(v) { [v] }
            print(len(wrap!(42)));
        )";
        REQUIRE(runCode(src) == "1\n");
    }

    SECTION("Macro used inside expression context") {
        std::string src = R"(
            macro inc(x) { x + 1 }
            var a = 10;
            var b = inc!(a);
            print(b);
        )";
        // inc!(a) expands to a + 1, which evaluates to 11.
        REQUIRE(runCode(src) == "11\n");
    }
}

TEST_CASE("Macros: lexer recognises 'macro' keyword", "[macros][lexer]") {
    SECTION("macro is a keyword, not an identifier") {
        Lexer lexer("macro");
        auto tokens = lexer.scanTokens();
        REQUIRE(tokens[0].type == TokenType::MACRO);
    }
}

TEST_CASE("Macros: error handling", "[macros]") {
    SECTION("Using a macro in a nested context") {
        std::string src = R"(
            macro negate(x) { -x }
            var v = negate!(5);
            print(v);
        )";
        REQUIRE(runCode(src) == "-5\n");
    }

    SECTION("Macro invocation inside another expression") {
        std::string src = R"(
            macro half(n) { n / 2 }
            print(half!(10) + half!(4));
        )";
        // half!(10) => 10/2 = 5, half!(4) => 4/2 = 2, sum = 7
        REQUIRE(runCode(src) == "7\n");
    }
}

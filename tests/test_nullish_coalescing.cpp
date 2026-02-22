#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

// Helper: run source and capture stdout
static std::string run(const std::string& source) {
    std::stringstream buf;
    std::streambuf* old = std::cout.rdbuf(buf.rdbuf());
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    auto program = parser.parse();
    Interpreter interp(source);
    interp.interpret(program);
    std::cout.rdbuf(old);
    return buf.str();
}

TEST_CASE("Nullish coalescing operator ??", "[nullish_coalescing]") {
    SECTION("Returns right side when left is nil") {
        REQUIRE(run("print(nil ?? 42);") == "42\n");
    }

    SECTION("Returns left side when left is not nil") {
        REQUIRE(run("print(10 ?? 42);") == "10\n");
    }

    SECTION("Returns left side when left is false (false is not nil)") {
        REQUIRE(run("print(false ?? true);") == "false\n");
    }

    SECTION("Returns left side when left is zero (0 is not nil)") {
        REQUIRE(run("print(0 ?? 99);") == "0\n");
    }

    SECTION("Returns left side when left is empty string") {
        REQUIRE(run("print(\"\" ?? \"default\");") == "\n");
    }

    SECTION("Returns left side when left is a string") {
        REQUIRE(run("print(\"hello\" ?? \"world\");") == "hello\n");
    }

    SECTION("Works with variables") {
        REQUIRE(run("var x = nil; print(x ?? \"fallback\");") == "fallback\n");
    }

    SECTION("Works with non-nil variable") {
        REQUIRE(run("var x = \"value\"; print(x ?? \"fallback\");") == "value\n");
    }

    SECTION("Right side not evaluated when left is not nil") {
        // If right side were evaluated it would throw; verify it isn't
        std::string source = "var x = 5; print(x ?? (1/0));";
        REQUIRE(run(source) == "5\n");
    }

    SECTION("Chains: first non-nil wins") {
        REQUIRE(run("print(nil ?? nil ?? 7);") == "7\n");
    }

    SECTION("Chains: stops at first non-nil") {
        REQUIRE(run("print(nil ?? 3 ?? 7);") == "3\n");
    }

    SECTION("Works with function return value") {
        std::string source = R"(
fn maybeNil(x) {
    if (x) { return "got it"; }
    return nil;
}
print(maybeNil(false) ?? "default");
)";
        REQUIRE(run(source) == "default\n");
    }

    SECTION("Works with function return value (non-nil)") {
        std::string source = R"(
fn maybeNil(x) {
    if (x) { return "got it"; }
    return nil;
}
print(maybeNil(true) ?? "default");
)";
        REQUIRE(run(source) == "got it\n");
    }

    SECTION("Lexer produces QUESTION_QUESTION token") {
        Lexer lexer("a ?? b");
        auto tokens = lexer.scanTokens();
        bool found = false;
        for (const auto& tok : tokens) {
            if (tok.type == TokenType::QUESTION_QUESTION) {
                found = true;
                break;
            }
        }
        REQUIRE(found);
    }
}

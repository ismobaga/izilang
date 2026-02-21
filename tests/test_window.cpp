#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

TEST_CASE("Native module system - window module", "[modules][window]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "window";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("std.window import syntax works") {
        std::string source = R"(
            import * as window from "std.window";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("window module has expected functions") {
        std::string source = R"(
            import * as window from "window";
            import * as assert from "std.assert";

            assert.ok(window.create != nil);
            assert.ok(window.destroy != nil);
            assert.ok(window.clear != nil);
            assert.ok(window.present != nil);
            assert.ok(window.pollEvent != nil);
            assert.ok(window.drawRect != nil);
            assert.ok(window.drawLine != nil);
            assert.ok(window.drawText != nil);
            assert.ok(window.setTitle != nil);
            assert.ok(window.getSize != nil);
            assert.ok(window.isOpen != nil);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("named imports from window module work") {
        std::string source = R"(
            import { create, destroy, clear, present, pollEvent } from "window";
            import * as assert from "std.assert";

            assert.ok(create != nil);
            assert.ok(destroy != nil);
            assert.ok(clear != nil);
            assert.ok(present != nil);
            assert.ok(pollEvent != nil);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("isOpen returns false for invalid handle") {
        std::string source = R"(
            import * as window from "window";
            import * as assert from "std.assert";

            var result = window.isOpen(9999);
            assert.eq(result, false);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

TEST_CASE("Native module system - http module", "[modules][http]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "http";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("std.http import syntax works") {
        std::string source = R"(
            import * as http from "std.http";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("http module has get, post, and request functions") {
        std::string source = R"(
            import * as http from "http";
            import * as assert from "std.assert";

            assert.ok(http.get != nil);
            assert.ok(http.post != nil);
            assert.ok(http.request != nil);
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
            import { get, post, request } from "http";
            import * as assert from "std.assert";

            assert.ok(get != nil);
            assert.ok(post != nil);
            assert.ok(request != nil);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("http.get() throws on invalid URL (missing scheme)") {
        std::string source = R"(
            import * as http from "http";
            http.get("not-a-url");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("http.get() throws on HTTPS URL") {
        std::string source = R"(
            import * as http from "http";
            http.get("https://example.com/");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("http.get() throws on unsupported scheme") {
        std::string source = R"(
            import * as http from "http";
            http.get("ftp://example.com/file.txt");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("http.get() throws when no argument given") {
        std::string source = R"(
            import * as http from "http";
            http.get();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("http.post() throws when called without required arguments") {
        std::string source = R"(
            import * as http from "http";
            http.post("http://example.com");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("http.request() throws when options map has no url") {
        std::string source = R"(
            import * as http from "http";
            http.request({"method": "GET"});
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("http.request() throws when argument is not a map") {
        std::string source = R"(
            import * as http from "http";
            http.request("not-a-map");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }
}

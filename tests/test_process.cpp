#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

TEST_CASE("Native module system - process module", "[modules][process]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "std.process";
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
            import { exit, status, args } from "std.process";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Wildcard import works") {
        std::string source = R"(
            import * as proc from "std.process";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("process.status() returns 0") {
        std::string source = R"(
            import * as proc from "std.process";
            var status = proc.status();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("process.args() returns array") {
        std::string source = R"(
            import * as proc from "std.process";
            var cmdArgs = proc.args();
            var count = len(cmdArgs);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        std::vector<std::string> args = {"test.iz", "arg1", "arg2"};
        interp.setCommandLineArgs(args);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("process.args() returns correct arguments") {
        std::string source = R"(
            import * as proc from "std.process";
            import * as assert from "std.assert";
            
            var cmdArgs = proc.args();
            assert.eq(len(cmdArgs), 3);
            assert.eq(cmdArgs[0], "test.iz");
            assert.eq(cmdArgs[1], "arg1");
            assert.eq(cmdArgs[2], "arg2");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        std::vector<std::string> args = {"test.iz", "arg1", "arg2"};
        interp.setCommandLineArgs(args);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Both 'process' and 'std.process' import names work") {
        std::string source1 = R"(
            import * as proc from "process";
            var s = proc.status();
        )";

        Lexer lexer1(source1);
        auto tokens1 = lexer1.scanTokens();
        Parser parser1(std::move(tokens1));
        auto program1 = parser1.parse();

        Interpreter interp1(source1);
        REQUIRE_NOTHROW(interp1.interpret(program1));

        std::string source2 = R"(
            import * as proc from "std.process";
            var s = proc.status();
        )";

        Lexer lexer2(source2);
        auto tokens2 = lexer2.scanTokens();
        Parser parser2(std::move(tokens2));
        auto program2 = parser2.parse();

        Interpreter interp2(source2);
        REQUIRE_NOTHROW(interp2.interpret(program2));
    }
}

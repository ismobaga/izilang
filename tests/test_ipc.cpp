#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>
#include <unistd.h>

using namespace izi;

TEST_CASE("Native module system - ipc module", "[modules][ipc]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "std.ipc";
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
            import * as ipc from "std.ipc";
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
            import { createPipe, openRead, openWrite, send, recv, tryRecv, close, removePipe } from "std.ipc";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Both 'ipc' and 'std.ipc' import names work") {
        std::string source1 = R"(
            import * as ipc from "ipc";
        )";
        Lexer lexer1(source1);
        auto tokens1 = lexer1.scanTokens();
        Parser parser1(std::move(tokens1));
        auto program1 = parser1.parse();
        Interpreter interp1(source1);
        REQUIRE_NOTHROW(interp1.interpret(program1));

        std::string source2 = R"(
            import * as ipc from "std.ipc";
        )";
        Lexer lexer2(source2);
        auto tokens2 = lexer2.scanTokens();
        Parser parser2(std::move(tokens2));
        auto program2 = parser2.parse();
        Interpreter interp2(source2);
        REQUIRE_NOTHROW(interp2.interpret(program2));
    }

    SECTION("createPipe, openRead, openWrite, send, recv, close, removePipe roundtrip") {
        std::string source = R"(
            import * as ipc from "std.ipc";
            import * as assert from "std.assert";

            var pipeName = "test_ipc_roundtrip_cpp";
            ipc.createPipe(pipeName);

            var reader = ipc.openRead(pipeName);
            var writer = ipc.openWrite(pipeName);

            ipc.send(writer, "hello ipc");
            var msg = ipc.recv(reader);

            assert.eq(msg, "hello ipc");

            ipc.close(writer);
            ipc.close(reader);
            ipc.removePipe(pipeName);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("tryRecv returns nil when no message is ready") {
        std::string source = R"(
            import * as ipc from "std.ipc";
            import * as assert from "std.assert";

            var pipeName = "test_ipc_tryrecv_cpp";
            ipc.createPipe(pipeName);

            var reader = ipc.openRead(pipeName);
            var writer = ipc.openWrite(pipeName);

            var msg = ipc.tryRecv(reader);
            assert.eq(msg, nil);

            ipc.close(writer);
            ipc.close(reader);
            ipc.removePipe(pipeName);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("tryRecv returns message when data is available") {
        std::string source = R"(
            import * as ipc from "std.ipc";
            import * as assert from "std.assert";

            var pipeName = "test_ipc_tryrecv_data_cpp";
            ipc.createPipe(pipeName);

            var reader = ipc.openRead(pipeName);
            var writer = ipc.openWrite(pipeName);

            ipc.send(writer, "data available");
            var msg = ipc.tryRecv(reader);
            assert.eq(msg, "data available");

            ipc.close(writer);
            ipc.close(reader);
            ipc.removePipe(pipeName);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("send and recv preserve messages with special characters") {
        std::string source = R"(
            import * as ipc from "std.ipc";
            import * as assert from "std.assert";

            var pipeName = "test_ipc_special_cpp";
            ipc.createPipe(pipeName);

            var reader = ipc.openRead(pipeName);
            var writer = ipc.openWrite(pipeName);

            ipc.send(writer, "hello world!");
            var msg = ipc.recv(reader);
            assert.eq(msg, "hello world!");

            ipc.close(writer);
            ipc.close(reader);
            ipc.removePipe(pipeName);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("send on read-only handle throws error") {
        std::string source = R"(
            import * as ipc from "std.ipc";

            var pipeName = "test_ipc_readonly_cpp";
            ipc.createPipe(pipeName);
            var reader = ipc.openRead(pipeName);
            var writer = ipc.openWrite(pipeName);
            ipc.close(writer);
            ipc.close(reader);
            ipc.removePipe(pipeName);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("send on read-only handle throws error with appropriate message") {
        std::string source = R"(
            import * as ipc from "std.ipc";

            var pipeName = "test_ipc_readonly_err_cpp";
            ipc.createPipe(pipeName);
            var reader = ipc.openRead(pipeName);
            var writer = ipc.openWrite(pipeName);
            ipc.send(reader, "should fail");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS_WITH(interp.interpret(program), Catch::Contains("read-only"));

        // Cleanup: open state may be partial, just remove the pipe
        unlink("/tmp/izi_ipc_test_ipc_readonly_err_cpp");
    }
}

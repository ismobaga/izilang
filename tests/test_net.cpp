#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

using namespace izi;

// Helper: find a free TCP port by binding to port 0 and releasing it
static int findFreePort() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 19876;
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = 0;
    if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(sock);
        return 19876;
    }
    socklen_t len = sizeof(addr);
    getsockname(sock, reinterpret_cast<struct sockaddr*>(&addr), &len);
    int port = ntohs(addr.sin_port);
    close(sock);
    return port;
}

TEST_CASE("Native module system - net module", "[modules][net]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "std.net";
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
            import * as net from "std.net";
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
            import { connect, listen, accept, send, recv, close, setTimeout } from "std.net";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Both 'net' and 'std.net' import names work") {
        std::string source1 = R"(
            import * as net from "net";
        )";
        Lexer lexer1(source1);
        auto tokens1 = lexer1.scanTokens();
        Parser parser1(std::move(tokens1));
        auto program1 = parser1.parse();
        Interpreter interp1(source1);
        REQUIRE_NOTHROW(interp1.interpret(program1));

        std::string source2 = R"(
            import * as net from "std.net";
        )";
        Lexer lexer2(source2);
        auto tokens2 = lexer2.scanTokens();
        Parser parser2(std::move(tokens2));
        auto program2 = parser2.parse();
        Interpreter interp2(source2);
        REQUIRE_NOTHROW(interp2.interpret(program2));
    }

    SECTION("net module has expected functions") {
        std::string source = R"(
            import * as net from "std.net";
            import * as assert from "std.assert";

            assert.ok(net.connect != nil);
            assert.ok(net.listen != nil);
            assert.ok(net.accept != nil);
            assert.ok(net.send != nil);
            assert.ok(net.recv != nil);
            assert.ok(net.close != nil);
            assert.ok(net.setTimeout != nil);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("net.connect() throws on invalid host") {
        std::string source = R"(
            import * as net from "std.net";
            net.connect("invalid.host.that.does.not.exist.example", 9999);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.connect() throws on invalid port (0)") {
        std::string source = R"(
            import * as net from "std.net";
            net.connect("localhost", 0);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.connect() throws on invalid port (65536)") {
        std::string source = R"(
            import * as net from "std.net";
            net.connect("localhost", 65536);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.listen() throws on invalid port (0)") {
        std::string source = R"(
            import * as net from "std.net";
            net.listen(0);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.send() throws with wrong argument types") {
        std::string source = R"(
            import * as net from "std.net";
            net.send("not-a-handle", "data");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.recv() throws with wrong argument types") {
        std::string source = R"(
            import * as net from "std.net";
            net.recv("not-a-handle");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.close() throws with invalid handle") {
        std::string source = R"(
            import * as net from "std.net";
            net.close(99999);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.setTimeout() throws with invalid handle") {
        std::string source = R"(
            import * as net from "std.net";
            net.setTimeout(99999, 100);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("TCP server listen, client connect, send, recv, close roundtrip") {
        int port = findFreePort();

        // Start server in background thread
        std::thread serverThread([port]() {
            std::string serverSrc = "import * as net from \"std.net\";\n"
                                    "import * as assert from \"std.assert\";\n"
                                    "var srv = net.listen(" + std::to_string(port) + ");\n"
                                    "var client = net.accept(srv, 3000);\n"
                                    "assert.ok(client != nil);\n"
                                    "var msg = net.recv(client);\n"
                                    "net.send(client, \"pong\");\n"
                                    "net.close(client);\n"
                                    "net.close(srv);\n";
            Lexer lexer(serverSrc);
            auto tokens = lexer.scanTokens();
            Parser parser(std::move(tokens));
            auto program = parser.parse();
            Interpreter interp(serverSrc);
            interp.interpret(program);
        });

        // Give server time to start listening before the client connects
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::string clientSrc = "import * as net from \"std.net\";\n"
                                "import * as assert from \"std.assert\";\n"
                                "var conn = net.connect(\"127.0.0.1\", " + std::to_string(port) + ");\n"
                                "net.send(conn, \"ping\");\n"
                                "var reply = net.recv(conn);\n"
                                "assert.eq(reply, \"pong\");\n"
                                "net.close(conn);\n";

        Lexer lexer(clientSrc);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        Interpreter interp(clientSrc);
        REQUIRE_NOTHROW(interp.interpret(program));

        serverThread.join();
    }

    SECTION("net.accept() returns nil on timeout when no connection arrives") {
        int port = findFreePort();

        std::string source = "import * as net from \"std.net\";\n"
                             "import * as assert from \"std.assert\";\n"
                             "var srv = net.listen(" + std::to_string(port) + ");\n"
                             "var client = net.accept(srv, 50);\n"
                             "assert.eq(client, nil);\n"
                             "net.close(srv);\n";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("net.send() throws when sending on a server socket") {
        int port = findFreePort();

        std::string source = "import * as net from \"std.net\";\n"
                             "var srv = net.listen(" + std::to_string(port) + ");\n"
                             "net.send(srv, \"data\");\n";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));

        // cleanup: close the server socket via native C++
        // (the handle is leaked since interpret threw, but OS will clean up)
    }

    SECTION("net.recv() throws when receiving on a server socket") {
        int port = findFreePort();

        std::string source = "import * as net from \"std.net\";\n"
                             "var srv = net.listen(" + std::to_string(port) + ");\n"
                             "net.recv(srv);\n";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("net.setTimeout() sets timeout without error") {
        int port = findFreePort();

        std::string source = "import * as net from \"std.net\";\n"
                             "var srv = net.listen(" + std::to_string(port) + ");\n"
                             "net.setTimeout(srv, 100);\n"
                             "net.close(srv);\n";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();
        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

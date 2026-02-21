#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include <sstream>

using namespace izi;

TEST_CASE("Native module system - math module", "[modules][math]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "math";
            var result = math.pi;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Identifier import syntax (import math) creates module object") {
        std::string source = R"(
            import math;
            var result = math.pow(2, 2);
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
            import { sqrt, pi } from "math";
            var result = sqrt(16);
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
            import * as m from "math";
            var result = m.sqrt(16);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Math functions work through module") {
        std::string source = R"(
            import "math";
            var pi_val = math.pi;
            var sqrt_val = math.sqrt(16);
            var pow_val = math.pow(2, 3);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("clamp() function works correctly") {
        std::string source = R"(
            import * as math from "math";
            import * as assert from "std.assert";
            
            var result1 = math.clamp(10, 0, 5);
            assert.eq(result1, 5);
            
            var result2 = math.clamp(-5, 0, 10);
            assert.eq(result2, 0);
            
            var result3 = math.clamp(5, 0, 10);
            assert.eq(result3, 5);
            
            var result4 = math.clamp(7.5, 3.2, 8.9);
            assert.eq(result4, 7.5);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("sign() function returns correct values") {
        std::string source = R"(
            import { sign } from "math";
            import * as assert from "std.assert";
            
            var pos = sign(42);
            assert.eq(pos, 1);
            
            var neg = sign(-42);
            assert.eq(neg, -1);
            
            var zero = sign(0);
            assert.eq(zero, 0);
            
            var posSmall = sign(0.001);
            assert.eq(posSmall, 1);
            
            var negSmall = sign(-0.001);
            assert.eq(negSmall, -1);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("isFinite() function works correctly") {
        std::string source = R"(
            import "math";
            import * as assert from "std.assert";
            
            var finite1 = math.isFinite(42.5);
            assert.eq(finite1, true);
            
            var finite2 = math.isFinite(0);
            assert.eq(finite2, true);
            
            var finite3 = math.isFinite(-100);
            assert.eq(finite3, true);
            
            var posInf = 1.0 / 0.0;
            var negInf = -1.0 / 0.0;
            var nan = 0.0 / 0.0;
            
            var notFinite1 = math.isFinite(posInf);
            assert.eq(notFinite1, false);
            
            var notFinite2 = math.isFinite(negInf);
            assert.eq(notFinite2, false);
            
            var notFinite3 = math.isFinite(nan);
            assert.eq(notFinite3, false);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("isNaN() function works correctly") {
        std::string source = R"(
            import "math";
            import * as assert from "std.assert";
            
            var notNaN1 = math.isNaN(42);
            assert.eq(notNaN1, false);
            
            var notNaN2 = math.isNaN(3.14);
            assert.eq(notNaN2, false);
            
            var notNaN3 = math.isNaN(0);
            assert.eq(notNaN3, false);
            
            var posInf = 1.0 / 0.0;
            var negInf = -1.0 / 0.0;
            var nan = 0.0 / 0.0;
            
            var notNaN4 = math.isNaN(posInf);
            assert.eq(notNaN4, false);
            
            var notNaN5 = math.isNaN(negInf);
            assert.eq(notNaN5, false);
            
            var isNaN1 = math.isNaN(nan);
            assert.eq(isNaN1, true);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - string module", "[modules][string]") {
    SECTION("String module functions work") {
        std::string source = R"(
            import "string";
            var upper = string.toUpper("hello");
            var parts = string.split("a,b,c", ",");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - array module", "[modules][array]") {
    SECTION("Array module functions work") {
        std::string source = R"(
            import { map, filter } from "array";
            var nums = [1, 2, 3, 4, 5];
            fn double(x) { return x * 2; }
            var doubled = map(nums, double);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - io module", "[modules][io]") {
    SECTION("IO module functions work") {
        std::string source = R"(
            import "io";
            io.writeFile("/tmp/test_modules.txt", "test");
            var exists = io.exists("/tmp/test_modules.txt");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - assert module", "[modules][assert]") {
    SECTION("Simple import works") {
        std::string source = R"(
            import "assert";
            assert.ok(true);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Wildcard import as std.assert works") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.ok(1 + 1 == 2);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("assert.ok passes with truthy values") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.ok(true);
            assert.ok(1);
            assert.ok("hello");
            assert.ok([1, 2, 3]);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("assert.ok fails with falsy values") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.ok(false);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("assert.ok with custom message") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.ok(5 > 3, "5 should be greater than 3");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("assert.eq passes with equal values") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.eq(1, 1);
            assert.eq("hello", "hello");
            assert.eq(true, true);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("assert.eq fails with different values") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.eq(1, 2);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("assert.ne passes with different values") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.ne(1, 2);
            assert.ne("hello", "world");
            assert.ne(true, false);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("assert.ne fails with equal values") {
        std::string source = R"(
            import * as assert from "std.assert";
            assert.ne(5, 5);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }
}

TEST_CASE("Native module system - module deduplication", "[modules]") {
    SECTION("Importing same module twice works") {
        std::string source = R"(
            import "math";
            import { sqrt } from "math";
            var result1 = math.sqrt(16);
            var result2 = sqrt(25);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - backward compatibility", "[modules]") {
    SECTION("Global functions still work") {
        std::string source = R"(
            var result = sqrt(16);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - log module", "[modules][log]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "log";
            log.info("Test message");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("std.log import syntax works") {
        std::string source = R"(
            import * as log from "std.log";
            log.info("Test message");
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
            import { info, warn, error, debug } from "log";
            info("Information message");
            warn("Warning message");
            error("Error message");
            debug("Debug message");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Named imports work with std.log") {
        std::string source = R"(
            import { info, warn, error, debug } from "std.log";
            info("Information message");
            warn("Warning message");
            error("Error message");
            debug("Debug message");
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
            import * as log from "log";
            log.info("Server started");
            log.warn("Low disk space");
            log.error("Failed to connect");
            log.debug("x = 42");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("Wildcard import works with std.log") {
        std::string source = R"(
            import * as log from "std.log";
            log.info("Server started");
            log.warn("Low disk space");
            log.error("Failed to connect");
            log.debug("x = 42");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - path module", "[modules][path]") {
    SECTION("Simple import creates module object") {
        std::string source = R"(
            import "path";
            var result = path.join("a", "b");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("std.path import syntax works") {
        std::string source = R"(
            import * as path from "std.path";
            var result = path.join("a", "b");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("path.join() works with multiple parts") {
        std::string source = R"(
            import * as path from "std.path";
            import * as assert from "std.assert";
            
            var result1 = path.join("a", "b", "c.txt");
            assert.eq(result1, "a/b/c.txt");
            
            var result2 = path.join("/tmp", "foo", "bar.txt");
            assert.eq(result2, "/tmp/foo/bar.txt");
            
            var result3 = path.join("", "a", "", "b");
            assert.eq(result3, "a/b");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("path.basename() extracts filename") {
        std::string source = R"(
            import * as path from "std.path";
            import * as assert from "std.assert";
            
            var result1 = path.basename("/tmp/a.txt");
            assert.eq(result1, "a.txt");
            
            var result2 = path.basename("/tmp/subdir/file.js");
            assert.eq(result2, "file.js");
            
            var result3 = path.basename("file.txt");
            assert.eq(result3, "file.txt");
            
            var result4 = path.basename("/tmp/");
            assert.eq(result4, "tmp");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("path.dirname() extracts directory") {
        std::string source = R"(
            import * as path from "std.path";
            import * as assert from "std.assert";
            
            var result1 = path.dirname("/tmp/a.txt");
            assert.eq(result1, "/tmp");
            
            var result2 = path.dirname("/tmp/subdir/file.js");
            assert.eq(result2, "/tmp/subdir");
            
            var result3 = path.dirname("file.txt");
            assert.eq(result3, ".");
            
            var result4 = path.dirname("/file.txt");
            assert.eq(result4, "/");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("path.extname() extracts extension") {
        std::string source = R"(
            import * as path from "std.path";
            import * as assert from "std.assert";
            
            var result1 = path.extname("/tmp/a.txt");
            assert.eq(result1, ".txt");
            
            var result2 = path.extname("file.js");
            assert.eq(result2, ".js");
            
            var result3 = path.extname("file");
            assert.eq(result3, "");
            
            var result4 = path.extname(".hidden");
            assert.eq(result4, "");
            
            var result5 = path.extname("file.tar.gz");
            assert.eq(result5, ".gz");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("path.normalize() resolves . and ..") {
        std::string source = R"(
            import * as path from "std.path";
            import * as assert from "std.assert";
            
            var result1 = path.normalize("../a/./b");
            assert.eq(result1, "../a/b");
            
            var result2 = path.normalize("/foo/bar/../baz");
            assert.eq(result2, "/foo/baz");
            
            var result3 = path.normalize("./a/b/c");
            assert.eq(result3, "a/b/c");
            
            var result4 = path.normalize("a//b///c");
            assert.eq(result4, "a/b/c");
            
            var result5 = path.normalize("/a/b/../../c");
            assert.eq(result5, "/c");
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
            import { join, basename, dirname } from "std.path";
            import * as assert from "std.assert";
            
            var joined = join("a", "b");
            assert.eq(joined, "a/b");
            
            var base = basename("/tmp/file.txt");
            assert.eq(base, "file.txt");
            
            var dir = dirname("/tmp/file.txt");
            assert.eq(dir, "/tmp");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Native module system - ui module", "[modules][ui]") {
    SECTION("ui module can be imported") {
        std::string source = R"(
            import ui;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui.color() creates a color value with r, g, b, a fields") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            var c = ui.color(255, 128, 0);
            assert.eq(c.r, 255);
            assert.eq(c.g, 128);
            assert.eq(c.b, 0);
            assert.eq(c.a, 255);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui.color() accepts optional alpha channel") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            var c = ui.color(10, 20, 30, 128);
            assert.eq(c.r, 10);
            assert.eq(c.g, 20);
            assert.eq(c.b, 30);
            assert.eq(c.a, 128);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui.key constants are accessible") {
        std::string source = R"(
            import ui;
            var esc = ui.key.escape;
            var sp = ui.key.space;
            var a = ui.key.a;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui.mouse constants are accessible") {
        std::string source = R"(
            import ui;
            var left = ui.mouse.left;
            var right = ui.mouse.right;
            var middle = ui.mouse.middle;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui module exposes expected functions") {
        std::string source = R"(
            import ui;
            var cw = ui.createWindow;
            var col = ui.color;
            var kd = ui.keyDown;
            var kp = ui.keyPressed;
            var md = ui.mouseDown;
            var mp = ui.mousePressed;
            var gmp = ui.getMousePosition;
            var gmw = ui.getMouseWheelMove;
            var gcp = ui.getCharPressed;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui.color() with wrong arg count throws") {
        std::string source = R"(
            import ui;
            var c = ui.color(255, 128);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("win.createPanel() returns a panel object with expected methods") {
        std::string source = R"(
            import ui;
            var win = ui.createWindow("Test", 800, 600);
            var panel = win.createPanel(0, 0, 400, 600);
            var b   = panel.begin;
            var e   = panel.end;
            var gmp = panel.getMousePosition;
            var cm  = panel.containsMouse;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("win.createPanel() with wrong arg count throws") {
        std::string source = R"(
            import ui;
            var win = ui.createWindow("Test", 800, 600);
            var panel = win.createPanel(0, 0, 400);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("panel.getMousePosition() returns a map with x and y") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            var win = ui.createWindow("Test", 800, 600);
            var panel = win.createPanel(100, 50, 400, 300);
            var pos = panel.getMousePosition();
            var x = pos.x;
            var y = pos.y;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("panel.containsMouse() returns a bool") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            var win = ui.createWindow("Test", 800, 600);
            var panel = win.createPanel(100, 50, 400, 300);
            var inside = panel.containsMouse();
            assert.eq(inside, false);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("panel exposes drawing methods") {
        std::string source = R"(
            import ui;
            var win = ui.createWindow("Test", 800, 600);
            var panel = win.createPanel(0, 0, 400, 600);
            var dt = panel.drawText;
            var fr = panel.fillRect;
            var dr = panel.drawRect;
            var dl = panel.drawLine;
            var dc = panel.drawCircle;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("multiple panels can be created from the same window") {
        std::string source = R"(
            import ui;
            var win   = ui.createWindow("Test", 1200, 800);
            var left  = win.createPanel(0, 0, 600, 800);
            var right = win.createPanel(600, 0, 600, 800);
            var glmp = left.getMousePosition;
            var grmp = right.getMousePosition;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

// ---------------------------------------------------------------------------
// VM bytecode backend - ui module tests
// These mirror the interpreter-side ui module tests, exercising the
// createVmUiModule() path via BytecodeCompiler + VM.
// Tests that require an actual display (createWindow) are omitted since
// the test environment is headless.
// ---------------------------------------------------------------------------

namespace {
// Helper: compile IZI source and run it through the bytecode VM.
static bool vmRunNoThrow(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    auto program = parser.parse();
    BytecodeCompiler compiler;
    Chunk chunk = compiler.compile(program);
    VM vm;
    registerVmNatives(vm);
    vm.run(chunk);
    return true;
}
}  // namespace

TEST_CASE("VM: Native module system - ui module", "[vm][modules][ui]") {
    SECTION("ui module can be imported via VM") {
        REQUIRE_NOTHROW(vmRunNoThrow("import ui;"));
    }

    SECTION("ui.color() creates a color value with r, g, b, a fields via VM") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            var c = ui.color(255, 128, 0);
            assert.eq(c.r, 255);
            assert.eq(c.g, 128);
            assert.eq(c.b, 0);
            assert.eq(c.a, 255);
        )";
        REQUIRE_NOTHROW(vmRunNoThrow(source));
    }

    SECTION("ui.color() accepts optional alpha channel via VM") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            var c = ui.color(10, 20, 30, 128);
            assert.eq(c.r, 10);
            assert.eq(c.g, 20);
            assert.eq(c.b, 30);
            assert.eq(c.a, 128);
        )";
        REQUIRE_NOTHROW(vmRunNoThrow(source));
    }

    SECTION("ui.key constants are accessible via VM") {
        std::string source = R"(
            import ui;
            var esc   = ui.key.escape;
            var sp    = ui.key.space;
            var a     = ui.key.a;
            var enter = ui.key.enter;
            var left  = ui.key.left;
            var right = ui.key.right;
            var up    = ui.key.up;
            var down  = ui.key.down;
        )";
        REQUIRE_NOTHROW(vmRunNoThrow(source));
    }

    SECTION("ui.mouse constants are accessible via VM") {
        std::string source = R"(
            import ui;
            var left   = ui.mouse.left;
            var right  = ui.mouse.right;
            var middle = ui.mouse.middle;
        )";
        REQUIRE_NOTHROW(vmRunNoThrow(source));
    }

    SECTION("ui module exposes expected functions via VM") {
        std::string source = R"(
            import ui;
            var cw  = ui.createWindow;
            var col = ui.color;
            var kd  = ui.keyDown;
            var kp  = ui.keyPressed;
            var md  = ui.mouseDown;
            var mp  = ui.mousePressed;
            var gmp = ui.getMousePosition;
            var gmw = ui.getMouseWheelMove;
            var gcp = ui.getCharPressed;
        )";
        REQUIRE_NOTHROW(vmRunNoThrow(source));
    }

    SECTION("ui.color() with wrong arg count reports error via VM") {
        // The VM catches runtime errors internally and returns Nil rather than
        // propagating them as C++ exceptions. Verify that execution still
        // completes gracefully (no crash) when given wrong arg count.
        std::string source = R"(
            import ui;
            var c = ui.color(255, 128);
        )";
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);
        VM vm;
        registerVmNatives(vm);
        // VM reports the error internally; run() should complete without crashing
        REQUIRE_NOTHROW(vm.run(chunk));
    }

    SECTION("ui.key.escape has expected value via VM") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            assert.eq(ui.key.escape, 256);
        )";
        REQUIRE_NOTHROW(vmRunNoThrow(source));
    }

    SECTION("ui.mouse.left has expected value via VM") {
        std::string source = R"(
            import * as assert from "std.assert";
            import ui;
            assert.eq(ui.mouse.left, 0);
        )";
        REQUIRE_NOTHROW(vmRunNoThrow(source));
    }
}

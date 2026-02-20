#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
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

#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "interp/native_modules.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "bytecode/vm_native_modules.hpp"

using namespace izi;

TEST_CASE("Image module - interpreter: module is registered", "[image][modules]") {
    SECTION("image is a native module") {
        REQUIRE(isNativeModule("image"));
        REQUIRE(isNativeModule("std.image"));
    }

    SECTION("import image creates module object") {
        std::string source = R"(
            import "image";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("import image exposes expected functions") {
        std::string source = R"(
            import "image";
            var hasLoad = image.load != nil;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.load() throws without raylib") {
        std::string source = R"(
            import "image";
            var img = image.load("test.png");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
#ifndef HAVE_RAYLIB
        REQUIRE_THROWS(interp.interpret(program));
#endif
    }

    SECTION("image.load() with wrong argument type throws") {
        std::string source = R"(
            import "image";
            var img = image.load(42);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }
}

TEST_CASE("Image module - VM: module is registered", "[image][modules][vm]") {
    SECTION("image is a native VM module") {
        REQUIRE(isVmNativeModule("image"));
        REQUIRE(isVmNativeModule("std.image"));
    }

    SECTION("import image in VM creates module object") {
        std::string source = R"(
            import "image";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        REQUIRE_NOTHROW(vm.run(chunk));
    }

    SECTION("VM image module exposes expected functions") {
        std::string source = R"(
            import "image";
            var hasLoad = image.load != nil;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        REQUIRE_NOTHROW(vm.run(chunk));
    }

    SECTION("VM image.load() with wrong argument type is handled") {
        std::string source = R"(
            import "image";
            var img = image.load(42);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        // VM catches runtime errors internally and returns Nil rather than re-throwing
        REQUIRE_NOTHROW(vm.run(chunk));
    }
}

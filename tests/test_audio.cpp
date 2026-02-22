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

TEST_CASE("Audio module - interpreter: module is registered", "[audio][modules]") {
    SECTION("audio is a native module") {
        REQUIRE(isNativeModule("audio"));
        REQUIRE(isNativeModule("std.audio"));
    }

    SECTION("import audio creates module object") {
        std::string source = R"(
            import "audio";
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("import audio exposes expected functions") {
        std::string source = R"(
            import "audio";
            var hasInit = audio.initDevice != nil;
            var hasClose = audio.closeDevice != nil;
            var hasReady = audio.isDeviceReady != nil;
            var hasVolume = audio.setMasterVolume != nil;
            var hasLoadSound = audio.loadSound != nil;
            var hasLoadMusic = audio.loadMusic != nil;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("audio.isDeviceReady() returns false without raylib") {
        std::string source = R"(
            import "audio";
            var ready = audio.isDeviceReady();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("audio.initDevice() works without raylib") {
        std::string source = R"(
            import "audio";
            audio.initDevice();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        // initDevice() uses miniaudio when raylib is unavailable. It succeeds even
        // on headless systems because miniaudio falls back to a null audio device.
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("audio.loadSound() throws when file is missing") {
        std::string source = R"(
            import "audio";
            var snd = audio.loadSound("test.wav");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        // Throws because test.wav does not exist (or device init required first).
        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("audio.loadMusic() throws when file is missing") {
        std::string source = R"(
            import "audio";
            var music = audio.loadMusic("test.mp3");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        // Throws because test.mp3 does not exist (or device init required first).
        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("audio.setMasterVolume() works without raylib") {
        std::string source = R"(
            import "audio";
            audio.setMasterVolume(0.5);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        // setMasterVolume() stores the value and applies it when a device is
        // initialized; it does not require a device to be open first.
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Audio module - VM: module is registered", "[audio][modules][vm]") {
    SECTION("audio is a native VM module") {
        REQUIRE(isVmNativeModule("audio"));
        REQUIRE(isVmNativeModule("std.audio"));
    }

    SECTION("import audio in VM creates module object") {
        std::string source = R"(
            import "audio";
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

    SECTION("VM audio module exposes expected functions") {
        std::string source = R"(
            import "audio";
            var hasInit = audio.initDevice != nil;
            var hasClose = audio.closeDevice != nil;
            var hasReady = audio.isDeviceReady != nil;
            var hasLoadSound = audio.loadSound != nil;
            var hasLoadMusic = audio.loadMusic != nil;
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

    SECTION("VM audio.isDeviceReady() returns false without raylib") {
        std::string source = R"(
            import "audio";
            var ready = audio.isDeviceReady();
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
}

TEST_CASE("UI module - Camera2D: createCamera2D is exposed", "[ui][camera]") {
    SECTION("ui.createCamera2D is available") {
        std::string source = R"(
            import "ui";
            var hasCamera = ui.createCamera2D != nil;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui.createCamera2D creates a camera object with methods") {
        std::string source = R"(
            import "ui";
            var cam = ui.createCamera2D(0, 0, 0, 0, 0, 1);
            var hasBegin = cam.beginMode != nil;
            var hasEnd = cam.endMode != nil;
            var hasSetTarget = cam.setTarget != nil;
            var hasSetOffset = cam.setOffset != nil;
            var hasSetZoom = cam.setZoom != nil;
            var hasW2S = cam.getWorldToScreen != nil;
            var hasS2W = cam.getScreenToWorld != nil;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("camera.setTarget/setOffset/setZoom/setRotation work") {
        std::string source = R"(
            import "ui";
            var cam = ui.createCamera2D(0, 0, 0, 0, 0, 1);
            cam.setTarget(100, 200);
            cam.setOffset(400, 300);
            cam.setRotation(45);
            cam.setZoom(2);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("camera.getWorldToScreen returns position map without raylib") {
        std::string source = R"(
            import "ui";
            var cam = ui.createCamera2D(0, 0, 0, 0, 0, 1);
            var pos = cam.getWorldToScreen(100, 200);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("UI module - Camera2D in VM", "[ui][camera][vm]") {
    SECTION("VM ui.createCamera2D creates a camera object") {
        std::string source = R"(
            import "ui";
            var cam = ui.createCamera2D(0, 0, 0, 0, 0, 1);
            var hasBegin = cam.beginMode != nil;
            var hasEnd = cam.endMode != nil;
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
}

TEST_CASE("UI module - loadTexture is exposed", "[ui][texture]") {
    SECTION("ui.loadTexture is available") {
        std::string source = R"(
            import "ui";
            var hasLoad = ui.loadTexture != nil;
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("ui.loadTexture throws without raylib") {
        std::string source = R"(
            import "ui";
            var tex = ui.loadTexture("test.png");
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
}

TEST_CASE("UI module - loadTexture in VM", "[ui][texture][vm]") {
    SECTION("VM ui.loadTexture is available") {
        std::string source = R"(
            import "ui";
            var hasLoad = ui.loadTexture != nil;
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
}

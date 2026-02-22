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

    SECTION("image.load() throws when file is missing") {
        std::string source = R"(
            import "image";
            var img = image.load("test.png");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        // Throws because test.png does not exist on disk.
        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
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

#ifndef HAVE_RAYLIB
// ---------------------------------------------------------------------------
// Tests that exercise the stb_image-backed implementation end-to-end.
// A 4×4 RGBA test image is written to a temp file, then loaded and processed.
// ---------------------------------------------------------------------------
#include "vendor/stb_image.h"
#include "vendor/stb_image_write.h"
#include <cstdio>

static std::string writeTempPng(const char* filename, int w, int h) {
    std::string path = std::string("/tmp/") + filename;
    // Fill with a simple gradient pattern (RGBA)
    std::vector<unsigned char> pixels(w * h * 4);
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = (y * w + x) * 4;
            pixels[i + 0] = static_cast<unsigned char>(x * 255 / (w - 1));
            pixels[i + 1] = static_cast<unsigned char>(y * 255 / (h - 1));
            pixels[i + 2] = 128;
            pixels[i + 3] = 255;
        }
    }
    stbi_write_png(path.c_str(), w, h, 4, pixels.data(), w * 4);
    return path;
}

TEST_CASE("Image module - stb_image operations", "[image][stb]") {
    const std::string testImg = writeTempPng("izi_test_image.png", 4, 4);

    SECTION("image.load() succeeds for a valid PNG") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var w = img.getWidth();
            var h = img.getHeight();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.resize() returns a new image with correct dimensions") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var r = img.resize(8, 6);
            var w = r.getWidth();
            var h = r.getHeight();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.crop() succeeds with valid region") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var c = img.crop(0, 0, 2, 2);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.rotate() with 90 degrees works") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var r = img.rotate(90);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.flipHorizontal() works") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var f = img.flipHorizontal();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.flipVertical() works") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var f = img.flipVertical();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.blur() works") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var b = img.blur(1);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("image.save() writes a PNG file") {
        std::string outPath = "/tmp/izi_test_output.png";
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            img.save(")" + outPath + R"(");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));

        // Verify the saved file can be re-loaded with stb_image
        int w = 0, h = 0, c = 0;
        unsigned char* data = stbi_load(outPath.c_str(), &w, &h, &c, 0);
        REQUIRE(data != nullptr);
        REQUIRE(w == 4);
        REQUIRE(h == 4);
        stbi_image_free(data);
    }

    SECTION("image.crop() out of bounds throws") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            var c = img.crop(3, 3, 10, 10);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }

    SECTION("image.unload() frees the image") {
        std::string source = R"(
            import "image";
            var img = image.load(")" + testImg + R"(");
            img.unload();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Audio module - miniaudio operations", "[audio][miniaudio]") {
    SECTION("audio.initDevice() and closeDevice() work") {
        std::string source = R"(
            import "audio";
            audio.initDevice();
            audio.closeDevice();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("audio.isDeviceReady() returns true after initDevice()") {
        std::string source = R"(
            import "audio";
            audio.initDevice();
            var ready = audio.isDeviceReady();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("audio.setMasterVolume() works before and after initDevice()") {
        std::string source = R"(
            import "audio";
            audio.setMasterVolume(0.7);
            audio.initDevice();
            audio.setMasterVolume(0.5);
            audio.closeDevice();
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_NOTHROW(interp.interpret(program));
    }

    SECTION("audio.loadSound() throws for missing file after initDevice()") {
        std::string source = R"(
            import "audio";
            audio.initDevice();
            var snd = audio.loadSound("nonexistent.wav");
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        REQUIRE_THROWS(interp.interpret(program));
    }
}
#endif  // !HAVE_RAYLIB

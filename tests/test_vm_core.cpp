#include "catch.hpp"
#include "bytecode/chunk.hpp"
#include "bytecode/opcode.hpp"
#include "bytecode/disassembler.hpp"
#include "bytecode/function.hpp"
#include "bytecode/vm.hpp"
#include <sstream>

using namespace izi;

// ============================================================
//  Core VM data-structure tests
// ============================================================

TEST_CASE("Chunk: write and line tracking", "[vm-core][chunk]") {
    Chunk chunk;

    SECTION("write stores bytes and line numbers") {
        chunk.write(static_cast<uint8_t>(OpCode::NIL), 1);
        chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

        REQUIRE(chunk.code.size() == 2);
        REQUIRE(chunk.lines.size() == 2);
        REQUIRE(chunk.getLine(0) == 1);
        REQUIRE(chunk.getLine(1) == 1);
    }

    SECTION("getLine returns 0 for out-of-range offset") {
        chunk.write(static_cast<uint8_t>(OpCode::NIL), 3);
        REQUIRE(chunk.getLine(99) == 0);
    }

    SECTION("constants are stored and retrievable") {
        size_t idx = chunk.addConstant(42.0);
        REQUIRE(idx == 0);
        REQUIRE(std::get<double>(chunk.constants[0]) == 42.0);
    }

    SECTION("names are stored and retrievable") {
        size_t idx = chunk.addName("myVar");
        REQUIRE(idx == 0);
        REQUIRE(chunk.names[0] == "myVar");
    }
}

// ============================================================
//  Function data-structure tests
// ============================================================

TEST_CASE("Function: core data structure", "[vm-core][function]") {
    SECTION("default constructor creates empty function") {
        Function fn;
        REQUIRE(fn.name.empty());
        REQUIRE(fn.params.empty());
        REQUIRE(fn.chunk != nullptr);
        REQUIRE(fn.arity() == 0);
    }

    SECTION("constructor stores name, params, and chunk") {
        auto chunk = std::make_shared<Chunk>();
        Function fn("myFunc", {"a", "b"}, chunk);
        REQUIRE(fn.name == "myFunc");
        REQUIRE(fn.params.size() == 2);
        REQUIRE(fn.arity() == 2);
        REQUIRE(fn.chunk.get() == chunk.get());
    }
}

// ============================================================
//  Disassembler tests
// ============================================================

TEST_CASE("Disassembler: simple instruction", "[vm-core][disassembler]") {
    Chunk chunk;
    chunk.write(static_cast<uint8_t>(OpCode::NIL), 1);
    chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

    std::ostringstream out;
    Disassembler::disassemble(chunk, "test", out);
    std::string result = out.str();

    REQUIRE(result.find("== test ==") != std::string::npos);
    REQUIRE(result.find("NIL") != std::string::npos);
    REQUIRE(result.find("RETURN") != std::string::npos);
}

TEST_CASE("Disassembler: CONSTANT instruction", "[vm-core][disassembler]") {
    Chunk chunk;
    uint8_t idx = static_cast<uint8_t>(chunk.addConstant(3.14));
    chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 2);
    chunk.write(idx, 2);
    chunk.write(static_cast<uint8_t>(OpCode::RETURN), 2);

    std::ostringstream out;
    Disassembler::disassemble(chunk, "constants", out);
    std::string result = out.str();

    REQUIRE(result.find("CONSTANT") != std::string::npos);
    REQUIRE(result.find("RETURN") != std::string::npos);
}

TEST_CASE("Disassembler: disassembleInstruction returns next offset", "[vm-core][disassembler]") {
    Chunk chunk;
    chunk.write(static_cast<uint8_t>(OpCode::NIL), 1);    // offset 0 — 1 byte
    chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1); // offset 1 — 1 byte

    std::ostringstream out;
    size_t next = Disassembler::disassembleInstruction(chunk, 0, out);
    REQUIRE(next == 1);
    next = Disassembler::disassembleInstruction(chunk, next, out);
    REQUIRE(next == 2);
}

// ============================================================
//  VM execution: load a definition and process at least one
//  instruction (success criterion from the issue).
// ============================================================

TEST_CASE("VM: load and execute CONSTANT instruction", "[vm-core][vm-execute]") {
    // Build a minimal chunk: push constant 42, return it.
    Chunk chunk;
    uint8_t idx = static_cast<uint8_t>(chunk.addConstant(42.0));
    chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1);
    chunk.write(idx, 1);
    chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

    VM vm;
    Value result = vm.run(chunk);

    REQUIRE(std::holds_alternative<double>(result));
    REQUIRE(std::get<double>(result) == 42.0);
}

TEST_CASE("VM: load and execute NIL instruction", "[vm-core][vm-execute]") {
    Chunk chunk;
    chunk.write(static_cast<uint8_t>(OpCode::NIL), 1);
    chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

    VM vm;
    Value result = vm.run(chunk);

    REQUIRE(std::holds_alternative<Nil>(result));
}

TEST_CASE("VM: arithmetic over two constants", "[vm-core][vm-execute]") {
    SECTION("addition") {
        Chunk chunk;
        uint8_t i10 = static_cast<uint8_t>(chunk.addConstant(10.0));
        uint8_t i5  = static_cast<uint8_t>(chunk.addConstant(5.0));
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i10, 1);
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i5,  1);
        chunk.write(static_cast<uint8_t>(OpCode::ADD), 1);
        chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);
        VM vm;
        Value result = vm.run(chunk);
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 15.0);
    }

    SECTION("subtraction") {
        Chunk chunk;
        uint8_t i10 = static_cast<uint8_t>(chunk.addConstant(10.0));
        uint8_t i3  = static_cast<uint8_t>(chunk.addConstant(3.0));
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i10, 1);
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i3,  1);
        chunk.write(static_cast<uint8_t>(OpCode::SUBTRACT), 1);
        chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);
        VM vm;
        Value result = vm.run(chunk);
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 7.0);
    }

    SECTION("multiplication") {
        Chunk chunk;
        uint8_t i4 = static_cast<uint8_t>(chunk.addConstant(4.0));
        uint8_t i5 = static_cast<uint8_t>(chunk.addConstant(5.0));
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i4, 1);
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i5, 1);
        chunk.write(static_cast<uint8_t>(OpCode::MULTIPLY), 1);
        chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);
        VM vm;
        Value result = vm.run(chunk);
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 20.0);
    }

    SECTION("comparison: greater than") {
        Chunk chunk;
        uint8_t i10 = static_cast<uint8_t>(chunk.addConstant(10.0));
        uint8_t i5  = static_cast<uint8_t>(chunk.addConstant(5.0));
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i10, 1);
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i5,  1);
        chunk.write(static_cast<uint8_t>(OpCode::GREATER), 1);
        chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);
        VM vm;
        Value result = vm.run(chunk);
        REQUIRE(std::holds_alternative<bool>(result));
        REQUIRE(std::get<bool>(result) == true);
    }
}

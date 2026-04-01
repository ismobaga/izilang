#include "catch.hpp"
#include "bytecode/chunk.hpp"
#include "bytecode/opcode.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "compile/compiler.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

// Helper to compile and run source code, returning the VM's final value.
static Value compileAndRun(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    auto program = parser.parse();
    BytecodeCompiler compiler;
    Chunk chunk = compiler.compile(program);
    VM vm;
    registerVmNatives(vm);
    return vm.run(chunk);
}

// Helper to capture stdout while running.
class OutputCapture {
   public:
    OutputCapture() : old_buf(std::cout.rdbuf()) { std::cout.rdbuf(buffer.rdbuf()); }
    ~OutputCapture() { std::cout.rdbuf(old_buf); }
    std::string get() { return buffer.str(); }

   private:
    std::stringstream buffer;
    std::streambuf* old_buf;
};

static std::string runAndCapture(const std::string& source) {
    OutputCapture capture;
    compileAndRun(source);
    return capture.get();
}

// ============================================================
//  BUILD_ARRAY: array literal construction
// ============================================================

TEST_CASE("VM BUILD_ARRAY: empty array", "[vm-complete][array]") {
    std::string source = R"(
        var a = [];
        print(a);
    )";
    REQUIRE(runAndCapture(source) == "[]\n");
}

TEST_CASE("VM BUILD_ARRAY: array literal with elements", "[vm-complete][array]") {
    std::string source = R"(
        var a = [1, 2, 3];
        print(a);
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("1") != std::string::npos);
    REQUIRE(out.find("2") != std::string::npos);
    REQUIRE(out.find("3") != std::string::npos);
}

TEST_CASE("VM BUILD_ARRAY: array indexing", "[vm-complete][array]") {
    std::string source = R"(
        var a = [10, 20, 30];
        print(a[0]);
        print(a[1]);
        print(a[2]);
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("10") != std::string::npos);
    REQUIRE(out.find("20") != std::string::npos);
    REQUIRE(out.find("30") != std::string::npos);
}

TEST_CASE("VM BUILD_ARRAY: nested arrays", "[vm-complete][array]") {
    std::string source = R"(
        var matrix = [[1, 2], [3, 4]];
        print(matrix[0][1]);
        print(matrix[1][0]);
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("2") != std::string::npos);
    REQUIRE(out.find("3") != std::string::npos);
}

TEST_CASE("VM BUILD_ARRAY: array set index", "[vm-complete][array]") {
    std::string source = R"(
        var a = [1, 2, 3];
        a[1] = 99;
        print(a[1]);
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("99") != std::string::npos);
}

// ============================================================
//  BUILD_MAP: map literal construction
// ============================================================

TEST_CASE("VM BUILD_MAP: empty map", "[vm-complete][map]") {
    std::string source = R"(
        var m = {};
        print(m);
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("{") != std::string::npos);
}

TEST_CASE("VM BUILD_MAP: map literal with entries", "[vm-complete][map]") {
    std::string source = R"(
        var m = {"name": "Alice", "age": 30};
        print(m["name"]);
        print(m["age"]);
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("Alice") != std::string::npos);
    REQUIRE(out.find("30") != std::string::npos);
}

TEST_CASE("VM BUILD_MAP: map property access with dot notation", "[vm-complete][map]") {
    std::string source = R"(
        var m = {"x": 42, "y": 7};
        print(m.x);
        print(m.y);
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("42") != std::string::npos);
    REQUIRE(out.find("7") != std::string::npos);
}

// ============================================================
//  Logical AND / OR (short-circuit)
// ============================================================

TEST_CASE("VM AND: short-circuit evaluation", "[vm-complete][logical]") {
    SECTION("true and true returns right") {
        std::string out = runAndCapture("print(true and true);");
        REQUIRE(out.find("true") != std::string::npos);
    }

    SECTION("false and true returns false (left)") {
        std::string out = runAndCapture("print(false and true);");
        REQUIRE(out.find("false") != std::string::npos);
    }

    SECTION("false and rhs not evaluated") {
        // If short-circuit is broken, division by zero would crash.
        REQUIRE_NOTHROW(compileAndRun("var x = false and (1/0);"));
    }

    SECTION("AND in if condition") {
        std::string out = runAndCapture("if (true and true) { print(\"yes\"); }");
        REQUIRE(out.find("yes") != std::string::npos);
    }

    SECTION("AND: false stops evaluation") {
        std::string out = runAndCapture("if (false and true) { print(\"yes\"); } else { print(\"no\"); }");
        REQUIRE(out.find("no") != std::string::npos);
    }
}

TEST_CASE("VM OR: short-circuit evaluation", "[vm-complete][logical]") {
    SECTION("true or false returns true (left)") {
        std::string out = runAndCapture("print(true or false);");
        REQUIRE(out.find("true") != std::string::npos);
    }

    SECTION("false or true returns true (right)") {
        std::string out = runAndCapture("print(false or true);");
        REQUIRE(out.find("true") != std::string::npos);
    }

    SECTION("true or rhs not evaluated") {
        REQUIRE_NOTHROW(compileAndRun("var x = true or (1/0);"));
    }

    SECTION("OR in if condition") {
        std::string out = runAndCapture("if (false or true) { print(\"yes\"); }");
        REQUIRE(out.find("yes") != std::string::npos);
    }

    SECTION("OR: false tries right side") {
        std::string out = runAndCapture("if (false or false) { print(\"yes\"); } else { print(\"no\"); }");
        REQUIRE(out.find("no") != std::string::npos);
    }
}

// ============================================================
//  Nullish coalescing (??)
// ============================================================

TEST_CASE("VM ??: nil gives right side", "[vm-complete][nullish]") {
    std::string out = runAndCapture("print(nil ?? 42);");
    REQUIRE(out.find("42") != std::string::npos);
}

TEST_CASE("VM ??: non-nil gives left side", "[vm-complete][nullish]") {
    std::string out = runAndCapture("print(10 ?? 42);");
    REQUIRE(out.find("10") != std::string::npos);
    REQUIRE(out.find("42") == std::string::npos);
}

TEST_CASE("VM ??: false is not nil", "[vm-complete][nullish]") {
    std::string out = runAndCapture("print(false ?? true);");
    REQUIRE(out.find("false") != std::string::npos);
}

TEST_CASE("VM ??: zero is not nil", "[vm-complete][nullish]") {
    std::string out = runAndCapture("print(0 ?? 99);");
    REQUIRE(out.find("0") != std::string::npos);
}

TEST_CASE("VM ??: with variable", "[vm-complete][nullish]") {
    SECTION("nil variable uses fallback") {
        std::string out = runAndCapture("var x = nil; print(x ?? \"fallback\");");
        REQUIRE(out.find("fallback") != std::string::npos);
    }

    SECTION("non-nil variable skips fallback") {
        std::string out = runAndCapture("var x = \"value\"; print(x ?? \"fallback\");");
        REQUIRE(out.find("value") != std::string::npos);
        REQUIRE(out.find("fallback") == std::string::npos);
    }
}

// ============================================================
//  FunctionExpr (lambda / anonymous functions)
// ============================================================

TEST_CASE("VM FunctionExpr: simple lambda", "[vm-complete][lambda]") {
    std::string source = R"(
        var add = fn(a, b) { return a + b; };
        print(add(3, 4));
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("7") != std::string::npos);
}

TEST_CASE("VM FunctionExpr: lambda assigned to variable", "[vm-complete][lambda]") {
    std::string source = R"(
        var square = fn(x) { return x * x; };
        print(square(5));
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("25") != std::string::npos);
}

TEST_CASE("VM FunctionExpr: lambda with no params", "[vm-complete][lambda]") {
    std::string source = R"(
        var greet = fn() { return "hello"; };
        print(greet());
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("hello") != std::string::npos);
}

TEST_CASE("VM FunctionExpr: lambda passed as argument", "[vm-complete][lambda]") {
    std::string source = R"(
        fn apply(f, x) { return f(x); }
        var double = fn(n) { return n * 2; };
        print(apply(double, 7));
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("14") != std::string::npos);
}

// ============================================================
//  Local variables in function scope
// ============================================================

TEST_CASE("VM local vars: declared inside function use local slots", "[vm-complete][locals]") {
    std::string source = R"(
        fn compute() {
            var x = 10;
            var y = 20;
            return x + y;
        }
        print(compute());
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("30") != std::string::npos);
}

TEST_CASE("VM local vars: multiple calls are independent", "[vm-complete][locals]") {
    // Each call should use fresh local slots; earlier calls must not pollute later ones.
    std::string source = R"(
        fn make(n) {
            var v = n * 2;
            return v;
        }
        print(make(3));
        print(make(5));
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("6") != std::string::npos);
    REQUIRE(out.find("10") != std::string::npos);
}

TEST_CASE("VM local vars: reassignment works", "[vm-complete][locals]") {
    std::string source = R"(
        fn counter() {
            var n = 0;
            n = n + 1;
            n = n + 1;
            return n;
        }
        print(counter());
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("2") != std::string::npos);
}

TEST_CASE("VM local vars: nested function inside function", "[vm-complete][locals]") {
    std::string source = R"(
        fn outer() {
            var x = 5;
            fn inner(y) { return y + 1; }
            return inner(x);
        }
        print(outer());
    )";
    std::string out = runAndCapture(source);
    REQUIRE(out.find("6") != std::string::npos);
}

// ============================================================
//  BUILD_ARRAY with direct VM opcodes (unit test)
// ============================================================

TEST_CASE("VM BUILD_ARRAY opcode: manual chunk", "[vm-complete][array][low-level]") {
    // Build chunk that creates [1.0, 2.0, 3.0] and returns it
    Chunk chunk;
    auto push_num = [&](double v) {
        uint8_t idx = static_cast<uint8_t>(chunk.addConstant(v));
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1);
        chunk.write(idx, 1);
    };
    push_num(1.0);
    push_num(2.0);
    push_num(3.0);
    chunk.write(static_cast<uint8_t>(OpCode::BUILD_ARRAY), 1);
    chunk.write(3, 1);
    chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

    VM vm;
    Value result = vm.run(chunk);
    REQUIRE(std::holds_alternative<std::shared_ptr<Array>>(result));
    auto arr = std::get<std::shared_ptr<Array>>(result);
    REQUIRE(arr->elements.size() == 3);
    REQUIRE(std::get<double>(arr->elements[0]) == 1.0);
    REQUIRE(std::get<double>(arr->elements[1]) == 2.0);
    REQUIRE(std::get<double>(arr->elements[2]) == 3.0);
}

TEST_CASE("VM BUILD_MAP opcode: manual chunk", "[vm-complete][map][low-level]") {
    // Build chunk that creates {"x": 10.0} and returns it
    Chunk chunk;
    // Push key "x"
    uint8_t keyIdx = static_cast<uint8_t>(chunk.addConstant(std::string("x")));
    chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1);
    chunk.write(keyIdx, 1);
    // Push value 10.0
    uint8_t valIdx = static_cast<uint8_t>(chunk.addConstant(10.0));
    chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1);
    chunk.write(valIdx, 1);
    // BUILD_MAP 1
    chunk.write(static_cast<uint8_t>(OpCode::BUILD_MAP), 1);
    chunk.write(1, 1);
    chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

    VM vm;
    Value result = vm.run(chunk);
    REQUIRE(std::holds_alternative<std::shared_ptr<Map>>(result));
    auto map = std::get<std::shared_ptr<Map>>(result);
    REQUIRE(map->entries.count("x") == 1);
    REQUIRE(std::get<double>(map->entries.at("x")) == 10.0);
}

TEST_CASE("VM JUMP_IF_NOT_NIL opcode: manual chunk", "[vm-complete][nullish][low-level]") {
    SECTION("non-nil value jumps over fallback") {
        // Mimics: 42 ?? 99  → should leave 42 on stack
        Chunk chunk;
        uint8_t i42 = static_cast<uint8_t>(chunk.addConstant(42.0));
        uint8_t i99 = static_cast<uint8_t>(chunk.addConstant(99.0));
        // Push 42
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i42, 1);
        // JUMP_IF_NOT_NIL over POP+CONSTANT(99)
        chunk.write(static_cast<uint8_t>(OpCode::JUMP_IF_NOT_NIL), 1);
        chunk.write(0); chunk.write(3);  // skip 3 bytes: POP + CONSTANT + index
        // POP
        chunk.write(static_cast<uint8_t>(OpCode::POP), 1);
        // CONSTANT 99
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i99, 1);
        // RETURN
        chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

        VM vm;
        Value result = vm.run(chunk);
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 42.0);
    }

    SECTION("nil value falls through to fallback") {
        // Mimics: nil ?? 99  → should leave 99 on stack
        Chunk chunk;
        uint8_t i99 = static_cast<uint8_t>(chunk.addConstant(99.0));
        // Push nil
        chunk.write(static_cast<uint8_t>(OpCode::NIL), 1);
        // JUMP_IF_NOT_NIL (does NOT jump because value is nil)
        chunk.write(static_cast<uint8_t>(OpCode::JUMP_IF_NOT_NIL), 1);
        chunk.write(0); chunk.write(3);  // offset 3: skip POP + CONSTANT + index
        // POP the nil
        chunk.write(static_cast<uint8_t>(OpCode::POP), 1);
        // CONSTANT 99
        chunk.write(static_cast<uint8_t>(OpCode::CONSTANT), 1); chunk.write(i99, 1);
        // RETURN
        chunk.write(static_cast<uint8_t>(OpCode::RETURN), 1);

        VM vm;
        Value result = vm.run(chunk);
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 99.0);
    }
}

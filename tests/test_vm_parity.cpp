#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"

#include <sstream>
#include <stdexcept>

using namespace izi;

static std::vector<StmtPtr> parseProgram(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    return parser.parse();
}

static std::string runWithInterpreter(const std::string& source) {
    std::ostringstream out;
    std::streambuf* old = std::cout.rdbuf(out.rdbuf());
    try {
        auto program = parseProgram(source);
        Interpreter interp(source);
        interp.interpret(program);
    } catch (...) {
        std::cout.rdbuf(old);
        throw;
    }
    std::cout.rdbuf(old);
    return out.str();
}

static std::string runWithVm(const std::string& source) {
    std::ostringstream out;
    std::streambuf* old = std::cout.rdbuf(out.rdbuf());
    try {
        auto program = parseProgram(source);
        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);
        VM vm;
        registerVmNatives(vm);
        (void)vm.run(chunk);
    } catch (...) {
        std::cout.rdbuf(old);
        throw;
    }
    std::cout.rdbuf(old);
    return out.str();
}

static void requireSameOutput(const std::string& source) {
    const std::string interpOut = runWithInterpreter(source);
    const std::string vmOut = runWithVm(source);
    REQUIRE(vmOut == interpOut);
}

TEST_CASE("VM parity: arithmetic and variables", "[vm-parity][p0]") {
    requireSameOutput(R"(
        var x = 10;
        var y = 3;
        print(x + y);
        print(x * y);
        print(x - y);
    )");
}

TEST_CASE("VM parity: control flow", "[vm-parity][p0]") {
    requireSameOutput(R"(
        var total = 0;
        for (var i = 0; i < 5; i = i + 1) {
            total = total + i;
        }
        if (total == 10) {
            print("ok");
        } else {
            print("bad");
        }
        print(total);
    )");
}

TEST_CASE("VM parity: arrays and maps", "[vm-parity][p0]") {
    requireSameOutput(R"(
        var a = [1, 2, 3];
        a[1] = 99;
        var m = {"name": "izi", "v": 4};
        print(a[1]);
        print(m.name);
        print(m["v"]);
    )");
}

TEST_CASE("VM known gap: functions and closures parity", "[vm-gap][!mayfail]") {
    requireSameOutput(R"(
        fn makeCounter() {
            var c = 0;
            return fn() {
                c = c + 1;
                return c;
            };
        }

        var inc = makeCounter();
        print(inc());
        print(inc());
    )");
}

TEST_CASE("VM parity: recursion", "[vm-parity][p0]") {
    requireSameOutput(R"(
        fn fact(n) {
            if (n <= 1) return 1;
            return n * fact(n - 1);
        }
        print(fact(5));
    )");
}

TEST_CASE("VM parity: nullish and logic", "[vm-parity][p0]") {
    requireSameOutput(R"(
        var a = nil;
        var b = 7;
        print(a ?? b);
        print(false ?? true);
        print(0 ?? 42);
        print(true and false);
        print(false or true);
    )");
}

TEST_CASE("VM known gap: try/catch/finally parity", "[vm-gap][!mayfail]") {
    requireSameOutput(R"(
        try {
            throw "boom";
        } catch (e) {
            print(e);
        } finally {
            print("done");
        }
    )");
}

TEST_CASE("VM known gap: class inheritance parity", "[vm-gap][!mayfail]") {
    requireSameOutput(R"(
        class A {
            fn speak() { return "A"; }
        }
        class B extends A {
            fn speak() { return super.speak() + "B"; }
        }
        var b = B();
        print(b.speak());
    )");
}

TEST_CASE("VM known gap: async parity", "[vm-gap][!mayfail]") {
    requireSameOutput(R"(
        async fn v() {
            return 42;
        }
        print(await v());
    )");
}

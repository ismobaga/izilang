#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "vm_test_utils.hpp"

using namespace izi;

TEST_CASE("VM Try-Catch Simple", "[vm-exception-simple]") {
    std::string source = R"(
        var executed = 0;
        try {
            throw "error";
            executed = 999;
        } catch(e) {
            executed = 1;
        }
        print(executed);
    )";

    std::string output = testutil::runVmSourceAndCapture(source, true);
    REQUIRE(output == "1\n");
}

TEST_CASE("VM Classes: Basic instantiation", "[vm-classes]") {
    SECTION("Empty class instantiation") {
        std::string source = R"(
            class Empty { }
            var e = Empty();
            print(e);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        testutil::OutputCapture capture;

        REQUIRE_NOTHROW(vm.run(chunk));

        std::string output = capture.output();
        REQUIRE(output.find("<Empty instance>") != std::string::npos);
    }
}

TEST_CASE("VM Classes: Property access", "[vm-classes]") {
    SECTION("Get and set properties") {
        std::string source = R"(
            class Counter {
                var count: Number;
            }
            var c = Counter();
            c.count = 5;
            print(c.count);
            c.count = c.count + 1;
            print(c.count);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        testutil::OutputCapture capture;

        REQUIRE_NOTHROW(vm.run(chunk));

        std::string output = capture.output();
        REQUIRE(output.find("5") != std::string::npos);
        REQUIRE(output.find("6") != std::string::npos);
    }
}

TEST_CASE("VM Classes: Methods", "[vm-classes]") {
    SECTION("Method call with this") {
        std::string source = R"(
            class Calculator {
                var value: Number;
                
                fn constructor(v: Number) {
                    this.value = v;
                }
                
                fn add(n: Number): Number {
                    this.value = this.value + n;
                    return this.value;
                }
                
                fn getValue(): Number {
                    return this.value;
                }
            }
            
            var calc = Calculator(10);
            print(calc.getValue());
            var result = calc.add(5);
            print(result);
            print(calc.getValue());
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        testutil::OutputCapture capture;

        REQUIRE_NOTHROW(vm.run(chunk));

        std::string output = capture.output();
        // Should print: 10, 15, 15
        size_t first_10 = output.find("10");
        REQUIRE(first_10 != std::string::npos);
        size_t first_15 = output.find("15", first_10);
        REQUIRE(first_15 != std::string::npos);
        size_t second_15 = output.find("15", first_15 + 2);
        REQUIRE(second_15 != std::string::npos);
    }
}

TEST_CASE("VM Classes: Multiple instances", "[vm-classes][vm-instances]") {
    SECTION("Multiple independent class instances") {
        std::string source = R"(
            class Counter {
                var count: Number;
                
                fn constructor(initial: Number) {
                    this.count = initial;
                }
                
                fn increment(): Void {
                    this.count = this.count + 1;
                }
            }
            
            var c1 = Counter(0);
            var c2 = Counter(100);
            
            c1.increment();
            c1.increment();
            c2.increment();
            
            print(c1.count);
            print(c2.count);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        testutil::OutputCapture capture;

        REQUIRE_NOTHROW(vm.run(chunk));

        std::string output = capture.output();
        REQUIRE(output.find("2") != std::string::npos);
        REQUIRE(output.find("101") != std::string::npos);
    }
}

TEST_CASE("VM Classes: Instance storage", "[vm-classes][vm-storage]") {
    SECTION("Store class instance in variable") {
        // Test basic class instance variable assignment
        std::string source = R"(
            class Point {
                var x: Number;
                var y: Number;
                
                fn constructor(x: Number, y: Number) {
                    this.x = x;
                    this.y = y;
                }
            }
            
            var p = Point(5, 10);
            print(p.x);
            print(p.y);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);
        testutil::OutputCapture capture;

        REQUIRE_NOTHROW(vm.run(chunk));

        std::string output = capture.output();
        REQUIRE(output.find("5") != std::string::npos);
        REQUIRE(output.find("10") != std::string::npos);
    }
}

TEST_CASE("VM Classes: Error handling", "[vm-classes][vm-errors]") {
    SECTION("Access undefined property - should not crash") {
        std::string source = R"(
            class Empty { }
            var e = Empty();
            print(e.nonexistent);
        )";

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), source);
        auto program = parser.parse();

        BytecodeCompiler compiler;
        Chunk chunk = compiler.compile(program);

        VM vm;
        registerVmNatives(vm);

        // VM may print error message but should handle it gracefully
        // The test verifies that accessing undefined properties doesn't crash
        bool test_passed = false;
        try {
            vm.run(chunk);
            test_passed = true;  // VM handled error gracefully
        } catch (const std::exception& e) {
            // Exception is expected for runtime errors
            std::string error_msg = e.what();
            // Verify error message mentions the undefined property
            if (error_msg.find("property") != std::string::npos || error_msg.find("nonexistent") != std::string::npos ||
                error_msg.find("Undefined") != std::string::npos) {
                test_passed = true;
            }
        }

        // Test passes if either VM handled it gracefully or threw appropriate error
        REQUIRE(test_passed);
    }
}

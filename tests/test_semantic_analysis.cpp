#include "catch.hpp"
#include "common/semantic_analyzer.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <iostream>

using namespace izi;

// Helper function to parse and analyze code
SemanticAnalyzer analyzeCode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), code);
    auto program = parser.parse();

    SemanticAnalyzer analyzer;
    analyzer.analyze(program);
    return analyzer;
}

// Helper to check if there are errors in diagnostics
bool hasErrors(const SemanticAnalyzer& analyzer) {
    for (const auto& diag : analyzer.getDiagnostics()) {
        if (diag.severity == SemanticDiagnostic::Severity::Error) {
            return true;
        }
    }
    return false;
}

// Helper to count errors
int countErrors(const SemanticAnalyzer& analyzer) {
    int count = 0;
    for (const auto& diag : analyzer.getDiagnostics()) {
        if (diag.severity == SemanticDiagnostic::Severity::Error) {
            count++;
        }
    }
    return count;
}

// Helper to check if error message contains substring
bool hasErrorContaining(const SemanticAnalyzer& analyzer, const std::string& substring) {
    for (const auto& diag : analyzer.getDiagnostics()) {
        if (diag.severity == SemanticDiagnostic::Severity::Error && diag.message.find(substring) != std::string::npos) {
            return true;
        }
    }
    return false;
}

TEST_CASE("Semantic Analysis: Type Checking - Variables", "[semantic][types]") {
    SECTION("Type mismatch: Number vs String") {
        std::string code = R"(
            var x: Number = "string";
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Type mismatch"));
        REQUIRE(hasErrorContaining(analyzer, "Number"));
        REQUIRE(hasErrorContaining(analyzer, "String"));
    }

    SECTION("Type mismatch: String vs Number") {
        std::string code = R"(
            var name: String = 42;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Type mismatch"));
    }

    SECTION("Type mismatch: Bool vs Number") {
        std::string code = R"(
            var flag: Bool = 100;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Type mismatch"));
    }

    SECTION("Correct type annotation") {
        std::string code = R"(
            var x: Number = 42;
            var y: String = "hello";
            var z: Bool = true;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Unannotated variables should not cause errors") {
        std::string code = R"(
            var x = "string";
            var y = 42;
            var z = true;
            var w = x;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Mixed annotated and unannotated") {
        std::string code = R"(
            var x = "test";
            var y: Number = 10;
            var z = y;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Class Validation - Duplicate Fields", "[semantic][classes]") {
    SECTION("Duplicate field names") {
        std::string code = R"(
            class Point {
                var x: Number;
                var y: Number;
                var x: String;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Duplicate field"));
        REQUIRE(hasErrorContaining(analyzer, "x"));
    }

    SECTION("No duplicate fields") {
        std::string code = R"(
            class Point {
                var x: Number;
                var y: Number;
                var z: Number;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Multiple classes with same field names (should be OK)") {
        std::string code = R"(
            class Point {
                var x: Number;
                var y: Number;
            }
            
            class Vector {
                var x: Number;
                var y: Number;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Class Validation - Duplicate Methods", "[semantic][classes]") {
    SECTION("Duplicate method names") {
        std::string code = R"(
            class Calculator {
                fn add(a: Number, b: Number): Number {
                    return a + b;
                }
                
                fn add(x: Number): Number {
                    return x;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Duplicate method"));
        REQUIRE(hasErrorContaining(analyzer, "add"));
    }

    SECTION("No duplicate methods") {
        std::string code = R"(
            class Calculator {
                fn add(a: Number, b: Number): Number {
                    return a + b;
                }
                
                fn subtract(a: Number, b: Number): Number {
                    return a - b;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Class Validation - This Usage", "[semantic][classes]") {
    SECTION("This used outside of method") {
        std::string code = R"(
            var x = this;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "this"));
        REQUIRE(hasErrorContaining(analyzer, "inside class methods"));
    }

    SECTION("This used in function (not method)") {
        std::string code = R"(
            fn test() {
                return this;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "this"));
    }

    SECTION("This used correctly in method") {
        std::string code = R"(
            class Point {
                var x: Number;
                
                fn getX(): Number {
                    return this.x;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Class Validation - Constructor Naming", "[semantic][classes]") {
    SECTION("Constructor named with class name") {
        std::string code = R"(
            class Point {
                var x: Number;
                
                fn Point(x: Number) {
                    this.x = x;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Constructor"));
        REQUIRE(hasErrorContaining(analyzer, "should be named 'constructor'"));
    }

    SECTION("Constructor named correctly") {
        std::string code = R"(
            class Point {
                var x: Number;
                
                fn constructor(x: Number) {
                    this.x = x;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Class without constructor") {
        std::string code = R"(
            class Empty {
                var value: Number;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Control Flow - Return Outside Function", "[semantic][control-flow]") {
    SECTION("Return in global scope") {
        std::string code = R"(
            return 42;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Return statement outside"));
    }

    SECTION("Return in function") {
        std::string code = R"(
            fn test() {
                return 42;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Control Flow - Break/Continue", "[semantic][control-flow]") {
    SECTION("Break outside loop") {
        std::string code = R"(
            break;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Break statement outside"));
    }

    SECTION("Continue outside loop") {
        std::string code = R"(
            continue;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Continue statement outside"));
    }

    SECTION("Break in while loop") {
        std::string code = R"(
            while (true) {
                break;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Continue in while loop") {
        std::string code = R"(
            while (true) {
                continue;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Complex Scenarios", "[semantic][integration]") {
    SECTION("Valid class with multiple features") {
        std::string code = R"(
            class Counter {
                var count: Number;
                var name: String;
                
                fn constructor(name: String) {
                    this.count = 0;
                    this.name = name;
                }
                
                fn increment(): Number {
                    this.count = this.count + 1;
                    return this.count;
                }
                
                fn getName(): String {
                    return this.name;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Multiple errors in one class") {
        std::string code = R"(
            class BadClass {
                var x: Number;
                var x: String;
                
                fn BadClass() {
                    this.x = 10;
                }
                
                fn method() {
                    return 1;
                }
                
                fn method() {
                    return 2;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(countErrors(analyzer) >= 3);  // At least duplicate field, wrong constructor name, duplicate method
    }

    SECTION("Function with type annotations") {
        std::string code = R"(
            fn add(a: Number, b: Number): Number {
                return a + b;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Function Call Validation", "[semantic][functions]") {
    SECTION("Function called with correct types") {
        std::string code = R"(
            fn add(a: Number, b: Number): Number {
                return a + b;
            }
            var result = add(10, 20);
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Function called with wrong argument count") {
        std::string code = R"(
            fn add(a: Number, b: Number): Number {
                return a + b;
            }
            var result = add(10);
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "expects 2 arguments"));
    }

    SECTION("Function called with wrong type") {
        std::string code = R"(
            fn greet(name: String): String {
                return name;
            }
            var result = greet(42);
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "parameter"));
        REQUIRE(hasErrorContaining(analyzer, "String"));
        REQUIRE(hasErrorContaining(analyzer, "Number"));
    }

    SECTION("Untyped function calls should not error") {
        std::string code = R"(
            fn flexible(x) {
                return x;
            }
            var a = flexible(10);
            var b = flexible("test");
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Advanced Type Checking", "[semantic][types][edge-cases]") {
    SECTION("Nil type accepts only nil") {
        std::string code = R"(
            var x: Nil = nil;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Any type accepts any value") {
        std::string code = R"(
            var a: Any = 42;
            var b: Any = "string";
            var c: Any = true;
            var d: Any = nil;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Void return type validation") {
        std::string code = R"(
            fn doNothing(): Void {
                var x = 10;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Array element type compatibility") {
        std::string code = R"(
            var nums: Array<Number> = [1, 2, 3];
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Map key-value type compatibility") {
        std::string code = R"(
            var data: Map<String,Number> = {"a": 1, "b": 2};
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Class Instance Scenarios", "[semantic][classes][edge-cases]") {
    SECTION("Multiple class instances") {
        std::string code = R"(
            class Point {
                var x: Number;
                var y: Number;
            }
            
            var p1 = Point();
            var p2 = Point();
            p1.x = 10;
            p2.x = 20;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Class instance passed to function") {
        std::string code = R"(
            class Point {
                var x: Number;
            }
            
            fn usePoint(p) {
                print(p.x);
            }
            
            var point = Point();
            usePoint(point);
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Nested property access") {
        std::string code = R"(
            class Inner {
                var value: Number;
            }
            
            class Outer {
                var inner: Inner;
            }
            
            var obj = Outer();
            var inner = Inner();
            obj.inner = inner;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

TEST_CASE("Semantic Analysis: Type Misuse Detection", "[semantic][types][errors]") {
    SECTION("Array type mismatch") {
        std::string code = R"(
            var nums: Array<Number> = "not an array";
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Type mismatch"));
    }

    SECTION("Map type mismatch") {
        std::string code = R"(
            var data: Map<String,Number> = 42;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Type mismatch"));
    }

    SECTION("Function type mismatch") {
        std::string code = R"(
            var callback: fn(Number) -> Number = "not a function";
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Type mismatch"));
    }
}

TEST_CASE("Semantic Analysis: Lowercase Type Annotations", "[semantic][types][lowercase]") {
    SECTION("Lowercase type names work for variables") {
        std::string code = R"(
            var x: number = 42;
            var y: string = "hello";
            var z: bool = true;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Lowercase type mismatch is detected") {
        std::string code = R"(
            var x: number = "not a number";
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE(hasErrors(analyzer));
        REQUIRE(hasErrorContaining(analyzer, "Type mismatch"));
    }

    SECTION("Issue example: fn with lowercase type annotations") {
        std::string code = R"(
            fn add(a: number, b: number): number {
                return a + b;
            }
            var name: string = "Alice";
            var age: number = 30;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Issue example: class with lowercase type annotations") {
        std::string code = R"(
            class Person {
                var name: string;
                var age: number;

                fn greet(): string {
                    return this.name;
                }
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Mixed case type annotations (both work)") {
        std::string code = R"(
            var a: Number = 1;
            var b: number = 2;
            var c: String = "hi";
            var d: string = "bye";
            var e: Bool = true;
            var f: bool = false;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Lowercase nil type annotation") {
        std::string code = R"(
            var x: nil = nil;
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Lowercase void return type") {
        std::string code = R"(
            fn doNothing(): void {
                var x = 10;
            }
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }

    SECTION("Lowercase any type") {
        std::string code = R"(
            var x: any = 42;
            var y: any = "string";
        )";

        auto analyzer = analyzeCode(code);
        REQUIRE_FALSE(hasErrors(analyzer));
    }
}

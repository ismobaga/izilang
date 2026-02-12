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
        if (diag.severity == SemanticDiagnostic::Severity::Error && 
            diag.message.find(substring) != std::string::npos) {
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
        REQUIRE(countErrors(analyzer) >= 3); // At least duplicate field, wrong constructor name, duplicate method
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

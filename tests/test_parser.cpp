#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "ast/stmt.hpp"
#include "ast/expr.hpp"

using namespace izi;

TEST_CASE("Parser handles type annotations on variables", "[parser]") {
    SECTION("Parse variable with Number type") {
        Lexer lexer("var x: Number = 42;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "x");
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse variable with String type") {
        Lexer lexer("var name: String = \"test\";");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "name");
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::String);
    }
    
    SECTION("Parse variable with Bool type") {
        Lexer lexer("var flag: Bool = true;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "flag");
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Bool);
    }
    
    SECTION("Parse variable with Array<Number> type") {
        Lexer lexer("var nums: Array<Number> = [1, 2, 3];");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "nums");
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Array);
        REQUIRE(varStmt->typeAnnotation->elementType != nullptr);
        REQUIRE(varStmt->typeAnnotation->elementType->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse variable with Map<String,Number> type") {
        Lexer lexer("var data: Map<String,Number> = {\"a\": 1};");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "data");
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Map);
        REQUIRE(varStmt->typeAnnotation->keyType != nullptr);
        REQUIRE(varStmt->typeAnnotation->keyType->kind == TypeAnnotation::Kind::String);
        REQUIRE(varStmt->typeAnnotation->valueType != nullptr);
        REQUIRE(varStmt->typeAnnotation->valueType->kind == TypeAnnotation::Kind::Number);
    }
}

TEST_CASE("Parser handles type annotations on functions", "[parser]") {
    SECTION("Parse function with parameter types") {
        Lexer lexer("fn add(a: Number, b: Number) { return a + b; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* funcStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(funcStmt != nullptr);
        REQUIRE(funcStmt->name == "add");
        REQUIRE(funcStmt->params.size() == 2);
        REQUIRE(funcStmt->paramTypes.size() == 2);
        REQUIRE(funcStmt->paramTypes[0] != nullptr);
        REQUIRE(funcStmt->paramTypes[0]->kind == TypeAnnotation::Kind::Number);
        REQUIRE(funcStmt->paramTypes[1] != nullptr);
        REQUIRE(funcStmt->paramTypes[1]->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse function with return type") {
        Lexer lexer("fn getNumber(): Number { return 42; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* funcStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(funcStmt != nullptr);
        REQUIRE(funcStmt->name == "getNumber");
        REQUIRE(funcStmt->returnType != nullptr);
        REQUIRE(funcStmt->returnType->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse function with both parameter and return types") {
        Lexer lexer("fn multiply(x: Number, y: Number): Number { return x * y; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* funcStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(funcStmt != nullptr);
        REQUIRE(funcStmt->name == "multiply");
        REQUIRE(funcStmt->params.size() == 2);
        REQUIRE(funcStmt->paramTypes.size() == 2);
        REQUIRE(funcStmt->paramTypes[0]->kind == TypeAnnotation::Kind::Number);
        REQUIRE(funcStmt->paramTypes[1]->kind == TypeAnnotation::Kind::Number);
        REQUIRE(funcStmt->returnType != nullptr);
        REQUIRE(funcStmt->returnType->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse function with Void return type") {
        Lexer lexer("fn log(msg: String): Void { print msg; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* funcStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(funcStmt != nullptr);
        REQUIRE(funcStmt->returnType != nullptr);
        REQUIRE(funcStmt->returnType->kind == TypeAnnotation::Kind::Void);
    }
}

TEST_CASE("Parser handles class declarations", "[parser]") {
    SECTION("Parse empty class") {
        Lexer lexer("class Point { }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* classStmt = dynamic_cast<ClassStmt*>(stmts[0].get());
        REQUIRE(classStmt != nullptr);
        REQUIRE(classStmt->name == "Point");
        REQUIRE(classStmt->fields.size() == 0);
        REQUIRE(classStmt->methods.size() == 0);
    }
    
    SECTION("Parse class with fields") {
        Lexer lexer("class Point { var x: Number; var y: Number; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* classStmt = dynamic_cast<ClassStmt*>(stmts[0].get());
        REQUIRE(classStmt != nullptr);
        REQUIRE(classStmt->name == "Point");
        REQUIRE(classStmt->fields.size() == 2);
        REQUIRE(classStmt->fields[0]->name == "x");
        REQUIRE(classStmt->fields[0]->typeAnnotation != nullptr);
        REQUIRE(classStmt->fields[0]->typeAnnotation->kind == TypeAnnotation::Kind::Number);
        REQUIRE(classStmt->fields[1]->name == "y");
        REQUIRE(classStmt->fields[1]->typeAnnotation != nullptr);
        REQUIRE(classStmt->fields[1]->typeAnnotation->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse class with methods") {
        Lexer lexer("class Point { fn getX(): Number { return this.x; } }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* classStmt = dynamic_cast<ClassStmt*>(stmts[0].get());
        REQUIRE(classStmt != nullptr);
        REQUIRE(classStmt->name == "Point");
        REQUIRE(classStmt->methods.size() == 1);
        REQUIRE(classStmt->methods[0]->name == "getX");
        REQUIRE(classStmt->methods[0]->returnType != nullptr);
        REQUIRE(classStmt->methods[0]->returnType->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse class with fields and methods") {
        Lexer lexer(R"(
            class Point {
                var x: Number;
                var y: Number;
                fn move(dx: Number, dy: Number): Void {
                    this.x = this.x + dx;
                    this.y = this.y + dy;
                }
            }
        )");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* classStmt = dynamic_cast<ClassStmt*>(stmts[0].get());
        REQUIRE(classStmt != nullptr);
        REQUIRE(classStmt->name == "Point");
        REQUIRE(classStmt->fields.size() == 2);
        REQUIRE(classStmt->methods.size() == 1);
        REQUIRE(classStmt->methods[0]->name == "move");
        REQUIRE(classStmt->methods[0]->params.size() == 2);
        REQUIRE(classStmt->methods[0]->paramTypes.size() == 2);
    }
    
    SECTION("Parse class with constructor") {
        Lexer lexer(R"(
            class Point {
                var x: Number;
                var y: Number;
                fn constructor(x: Number, y: Number) {
                    this.x = x;
                    this.y = y;
                }
            }
        )");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* classStmt = dynamic_cast<ClassStmt*>(stmts[0].get());
        REQUIRE(classStmt != nullptr);
        REQUIRE(classStmt->name == "Point");
        REQUIRE(classStmt->fields.size() == 2);
        REQUIRE(classStmt->methods.size() == 1);
        REQUIRE(classStmt->methods[0]->name == "constructor");
    }
}

TEST_CASE("Parser handles property access and this keyword", "[parser]") {
    SECTION("Parse property access") {
        Lexer lexer("var value = obj.property;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->initializer != nullptr);
        auto* propExpr = dynamic_cast<PropertyExpr*>(varStmt->initializer.get());
        REQUIRE(propExpr != nullptr);
        REQUIRE(propExpr->property == "property");
    }
    
    SECTION("Parse property assignment") {
        Lexer lexer("obj.property = 42;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* exprStmt = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(exprStmt != nullptr);
        auto* setPropExpr = dynamic_cast<SetPropertyExpr*>(exprStmt->expr.get());
        REQUIRE(setPropExpr != nullptr);
        REQUIRE(setPropExpr->property == "property");
    }
    
    SECTION("Parse this keyword") {
        Lexer lexer("var self = this;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->initializer != nullptr);
        auto* thisExpr = dynamic_cast<ThisExpr*>(varStmt->initializer.get());
        REQUIRE(thisExpr != nullptr);
    }
    
    SECTION("Parse this.property access") {
        Lexer lexer("var value = this.x;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->initializer != nullptr);
        auto* propExpr = dynamic_cast<PropertyExpr*>(varStmt->initializer.get());
        REQUIRE(propExpr != nullptr);
        REQUIRE(propExpr->property == "x");
        auto* thisExpr = dynamic_cast<ThisExpr*>(propExpr->object.get());
        REQUIRE(thisExpr != nullptr);
    }
    
    SECTION("Parse this.property assignment") {
        Lexer lexer("this.x = 10;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* exprStmt = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(exprStmt != nullptr);
        auto* setPropExpr = dynamic_cast<SetPropertyExpr*>(exprStmt->expr.get());
        REQUIRE(setPropExpr != nullptr);
        REQUIRE(setPropExpr->property == "x");
        auto* thisExpr = dynamic_cast<ThisExpr*>(setPropExpr->object.get());
        REQUIRE(thisExpr != nullptr);
    }
    
    SECTION("Parse chained property access") {
        Lexer lexer("var value = obj.prop1.prop2;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->initializer != nullptr);
        auto* propExpr2 = dynamic_cast<PropertyExpr*>(varStmt->initializer.get());
        REQUIRE(propExpr2 != nullptr);
        REQUIRE(propExpr2->property == "prop2");
        auto* propExpr1 = dynamic_cast<PropertyExpr*>(propExpr2->object.get());
        REQUIRE(propExpr1 != nullptr);
        REQUIRE(propExpr1->property == "prop1");
    }
}

TEST_CASE("Parser handles function type annotations", "[parser]") {
    SECTION("Parse function type with no params") {
        Lexer lexer("var callback: fn() -> Number;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Function);
        REQUIRE(varStmt->typeAnnotation->paramTypes.size() == 0);
        REQUIRE(varStmt->typeAnnotation->valueType != nullptr);
        REQUIRE(varStmt->typeAnnotation->valueType->kind == TypeAnnotation::Kind::Number);
    }
    
    SECTION("Parse function type with params") {
        Lexer lexer("var op: fn(Number, Number) -> Number;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Function);
        REQUIRE(varStmt->typeAnnotation->paramTypes.size() == 2);
        REQUIRE(varStmt->typeAnnotation->paramTypes[0]->kind == TypeAnnotation::Kind::Number);
        REQUIRE(varStmt->typeAnnotation->paramTypes[1]->kind == TypeAnnotation::Kind::Number);
        REQUIRE(varStmt->typeAnnotation->valueType->kind == TypeAnnotation::Kind::Number);
    }
}

#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "ast/stmt.hpp"
#include "ast/expr.hpp"

using namespace izi;

TEST_CASE("Parser handles optional semicolons", "[parser][semicolon]") {
    
    SECTION("Single variable declaration without semicolon") {
        Lexer lexer("var x = 42");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "x");
    }
    
    SECTION("Multiple statements on separate lines without semicolons") {
        Lexer lexer("var x = 42\nvar y = 3.14");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 2);
        auto* varStmt1 = dynamic_cast<VarStmt*>(stmts[0].get());
        auto* varStmt2 = dynamic_cast<VarStmt*>(stmts[1].get());
        REQUIRE(varStmt1 != nullptr);
        REQUIRE(varStmt2 != nullptr);
        REQUIRE(varStmt1->name == "x");
        REQUIRE(varStmt2->name == "y");
    }
    
    SECTION("Single variable declaration with semicolon") {
        Lexer lexer("var x = 42;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "x");
    }
    
    SECTION("Multiple statements with semicolons") {
        Lexer lexer("var x = 42;\nvar y = 3.14;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 2);
        auto* varStmt1 = dynamic_cast<VarStmt*>(stmts[0].get());
        auto* varStmt2 = dynamic_cast<VarStmt*>(stmts[1].get());
        REQUIRE(varStmt1 != nullptr);
        REQUIRE(varStmt2 != nullptr);
        REQUIRE(varStmt1->name == "x");
        REQUIRE(varStmt2->name == "y");
    }
    
    SECTION("Multiple statements on one line with semicolons") {
        Lexer lexer("var x = 42; var y = 3.14;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 2);
        auto* varStmt1 = dynamic_cast<VarStmt*>(stmts[0].get());
        auto* varStmt2 = dynamic_cast<VarStmt*>(stmts[1].get());
        REQUIRE(varStmt1 != nullptr);
        REQUIRE(varStmt2 != nullptr);
        REQUIRE(varStmt1->name == "x");
        REQUIRE(varStmt2->name == "y");
    }
    
    SECTION("Multiple statements on one line without semicolons should throw error") {
        Lexer lexer("var x = 42 var y = 3.14");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        // The parser should have encountered an error during parsing
        // which results in a nullptr statement being added to the list
        bool hasError = false;
        for (const auto& stmt : stmts) {
            if (stmt == nullptr) {
                hasError = true;
                break;
            }
        }
        REQUIRE(hasError);
    }
    
    SECTION("Mixed styles - some with, some without") {
        Lexer lexer("var x = 42\nvar y = 3.14;\nvar z = 2.71");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 3);
        auto* varStmt1 = dynamic_cast<VarStmt*>(stmts[0].get());
        auto* varStmt2 = dynamic_cast<VarStmt*>(stmts[1].get());
        auto* varStmt3 = dynamic_cast<VarStmt*>(stmts[2].get());
        REQUIRE(varStmt1 != nullptr);
        REQUIRE(varStmt2 != nullptr);
        REQUIRE(varStmt3 != nullptr);
        REQUIRE(varStmt1->name == "x");
        REQUIRE(varStmt2->name == "y");
        REQUIRE(varStmt3->name == "z");
    }
    
    SECTION("Return statement without semicolon") {
        Lexer lexer("fn test() {\nreturn 42\n}");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* fnStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fnStmt != nullptr);
    }
    
    SECTION("Return statement with semicolon") {
        Lexer lexer("fn test() {\nreturn 42;\n}");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* fnStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fnStmt != nullptr);
    }
    
    SECTION("Break without semicolon") {
        Lexer lexer("while (true) {\nbreak\n}");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
    }
    
    SECTION("Continue without semicolon") {
        Lexer lexer("while (true) {\ncontinue\n}");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
    }
    
    SECTION("Expression statement without semicolon") {
        Lexer lexer("42 + 3");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* exprStmt = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(exprStmt != nullptr);
    }
    
    SECTION("Expression statement with semicolon") {
        Lexer lexer("42 + 3;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* exprStmt = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(exprStmt != nullptr);
    }
    
    SECTION("Three statements on one line with proper semicolons") {
        Lexer lexer("var a = 1; var b = 2; var c = 3;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 3);
    }
}

TEST_CASE("Parser handles optional semicolons in complex scenarios", "[parser][semicolon]") {
    
    SECTION("Function with multiple statements without semicolons") {
        Lexer lexer(R"(
fn add(a, b) {
    var sum = a + b
    return sum
}
)");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* fnStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fnStmt != nullptr);
        REQUIRE(fnStmt->body.size() == 2);
    }
    
    SECTION("If statement without semicolons") {
        Lexer lexer(R"(
if (true) {
    var x = 1
    var y = 2
}
)");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
        REQUIRE(ifStmt != nullptr);
    }
    
    SECTION("While loop without semicolons") {
        Lexer lexer(R"(
var i = 0
while (i < 10) {
    i = i + 1
}
)");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 2);
    }
    
    SECTION("Class with fields without semicolons") {
        Lexer lexer(R"(
class Point {
    var x
    var y
}
)");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();
        
        REQUIRE(stmts.size() == 1);
        auto* classStmt = dynamic_cast<ClassStmt*>(stmts[0].get());
        REQUIRE(classStmt != nullptr);
    }
}

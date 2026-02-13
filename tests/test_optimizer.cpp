#include "catch.hpp"
#include "compile/optimizer.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <sstream>

using namespace izi;

// Helper function to parse code
std::vector<StmtPtr> parseCode(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), code);
    return parser.parse();
}

TEST_CASE("Optimizer: Constant folding for binary expressions", "[optimizer][constant-folding]") {
    SECTION("Addition") {
        auto program = parseCode("var x = 2 + 3;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        REQUIRE(optimized.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "x");
        
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(literal != nullptr);
        REQUIRE(std::holds_alternative<double>(literal->value));
        REQUIRE(std::get<double>(literal->value) == 5.0);
    }
    
    SECTION("Subtraction") {
        auto program = parseCode("var x = 10 - 4;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == 6.0);
    }
    
    SECTION("Multiplication") {
        auto program = parseCode("var x = 5 * 6;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == 30.0);
    }
    
    SECTION("Division") {
        auto program = parseCode("var x = 20 / 4;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == 5.0);
    }
    
    SECTION("Comparison operators") {
        auto program = parseCode("var x = 5 > 3;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::holds_alternative<bool>(literal->value));
        REQUIRE(std::get<bool>(literal->value) == true);
    }
}

TEST_CASE("Optimizer: Constant folding for unary expressions", "[optimizer][constant-folding]") {
    SECTION("Negation") {
        auto program = parseCode("var x = -5;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::get<double>(literal->value) == -5.0);
    }
    
    SECTION("Logical NOT") {
        auto program = parseCode("var x = !false;");
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
        auto* literal = dynamic_cast<LiteralExpr*>(varStmt->initializer.get());
        REQUIRE(std::holds_alternative<bool>(literal->value));
        REQUIRE(std::get<bool>(literal->value) == true);
    }
}

TEST_CASE("Optimizer: Dead code elimination after return", "[optimizer][dead-code]") {
    auto program = parseCode(R"(
        fn test() {
            print("before");
            return 42;
            print("after");
        }
    )");
    
    Optimizer optimizer;
    auto optimized = optimizer.optimize(std::move(program));
    
    REQUIRE(optimized.size() == 1);
    auto* funcStmt = dynamic_cast<FunctionStmt*>(optimized[0].get());
    REQUIRE(funcStmt != nullptr);
    
    // The function body should have 2 statements, not 3
    // (print and return, but not the unreachable print)
    REQUIRE(funcStmt->body.size() == 2);
}

TEST_CASE("Optimizer: Dead code elimination in if statements", "[optimizer][dead-code]") {
    SECTION("Constant true condition") {
        auto program = parseCode(R"(
            if (true) {
                print("then");
            } else {
                print("else");
            }
        )");
        
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        REQUIRE(optimized.size() == 1);
        // The if statement should be replaced with just the then branch
        auto* blockStmt = dynamic_cast<BlockStmt*>(optimized[0].get());
        REQUIRE(blockStmt != nullptr);
    }
    
    SECTION("Constant false condition") {
        auto program = parseCode(R"(
            if (false) {
                print("then");
            } else {
                print("else");
            }
        )");
        
        Optimizer optimizer;
        auto optimized = optimizer.optimize(std::move(program));
        
        REQUIRE(optimized.size() == 1);
        // The if statement should be replaced with the else branch
        auto* blockStmt = dynamic_cast<BlockStmt*>(optimized[0].get());
        REQUIRE(blockStmt != nullptr);
    }
}

TEST_CASE("Optimizer: Dead code elimination in while loops", "[optimizer][dead-code]") {
    auto program = parseCode(R"(
        while (false) {
            print("never");
        }
    )");
    
    Optimizer optimizer;
    auto optimized = optimizer.optimize(std::move(program));
    
    // The while loop with a constant false condition should be eliminated
    REQUIRE(optimized.size() == 0);
}

TEST_CASE("Optimizer: Non-constant expressions are not folded", "[optimizer]") {
    auto program = parseCode("var x = a + b;");
    Optimizer optimizer;
    auto optimized = optimizer.optimize(std::move(program));
    
    auto* varStmt = dynamic_cast<VarStmt*>(optimized[0].get());
    REQUIRE(varStmt != nullptr);
    
    // Should still be a binary expression, not a literal
    auto* binaryExpr = dynamic_cast<BinaryExpr*>(varStmt->initializer.get());
    REQUIRE(binaryExpr != nullptr);
}

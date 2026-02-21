#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "ast/stmt.hpp"
#include "ast/expr.hpp"
#include "common/diagnostics.hpp"

using namespace izi;

// ---------------------------------------------------------------------------
// Helper: lex + parse a source string and return the statement list
// ---------------------------------------------------------------------------
static std::vector<StmtPtr> parseSource(const std::string& src) {
    Lexer lexer(src);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens));
    return parser.parse();
}

// ===========================================================================
// Core syntax tests (functions, blocks, if/while, return, expressions)
// ===========================================================================

TEST_CASE("Parser handles basic function declarations", "[parser][core]") {
    SECTION("Parse function with no parameters") {
        auto stmts = parseSource("fn greet() { }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->name == "greet");
        REQUIRE(fn->params.empty());
        REQUIRE(fn->body.empty());
    }

    SECTION("Parse function with multiple parameters") {
        auto stmts = parseSource("fn add(a, b, c) { return a + b + c; }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->name == "add");
        REQUIRE(fn->params.size() == 3);
        REQUIRE(fn->params[0] == "a");
        REQUIRE(fn->params[1] == "b");
        REQUIRE(fn->params[2] == "c");
    }

    SECTION("Parse function body with statements") {
        auto stmts = parseSource("fn double(x) { var result = x * 2; return result; }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->body.size() == 2);
        // First statement is a VarStmt
        REQUIRE(dynamic_cast<VarStmt*>(fn->body[0].get()) != nullptr);
        // Second statement is a ReturnStmt
        REQUIRE(dynamic_cast<ReturnStmt*>(fn->body[1].get()) != nullptr);
    }

    SECTION("Parse anonymous function expression") {
        auto stmts = parseSource("var f = fn(x) { return x; };");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        auto* fnExpr = dynamic_cast<FunctionExpr*>(var->initializer.get());
        REQUIRE(fnExpr != nullptr);
        REQUIRE(fnExpr->params.size() == 1);
        REQUIRE(fnExpr->params[0] == "x");
    }
}

TEST_CASE("Parser handles block statements", "[parser][core]") {
    SECTION("Parse empty block") {
        auto stmts = parseSource("fn f() { { } }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->body.size() == 1);
        auto* block = dynamic_cast<BlockStmt*>(fn->body[0].get());
        REQUIRE(block != nullptr);
        REQUIRE(block->statements.empty());
    }

    SECTION("Parse block with multiple statements") {
        auto stmts = parseSource("fn f() { var a = 1; var b = 2; }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->body.size() == 2);
    }
}

TEST_CASE("Parser handles if statements", "[parser][core]") {
    SECTION("Parse simple if statement") {
        auto stmts = parseSource("if (x > 0) { var y = 1; }");
        REQUIRE(stmts.size() == 1);
        auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
        REQUIRE(ifStmt != nullptr);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->thenBranch != nullptr);
        REQUIRE(ifStmt->elseBranch == nullptr);
    }

    SECTION("Parse if-else statement") {
        auto stmts = parseSource("if (x) { var a = 1; } else { var b = 2; }");
        REQUIRE(stmts.size() == 1);
        auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
        REQUIRE(ifStmt != nullptr);
        REQUIRE(ifStmt->thenBranch != nullptr);
        REQUIRE(ifStmt->elseBranch != nullptr);
    }

    SECTION("Parse if-else-if chain") {
        auto stmts = parseSource("if (a) { } else if (b) { } else { }");
        REQUIRE(stmts.size() == 1);
        auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
        REQUIRE(ifStmt != nullptr);
        // else branch is another IfStmt
        auto* elseIf = dynamic_cast<IfStmt*>(ifStmt->elseBranch.get());
        REQUIRE(elseIf != nullptr);
        REQUIRE(elseIf->elseBranch != nullptr);
    }

    SECTION("Parse if condition is comparison expression") {
        auto stmts = parseSource("if (x == 42) { }");
        REQUIRE(stmts.size() == 1);
        auto* ifStmt = dynamic_cast<IfStmt*>(stmts[0].get());
        REQUIRE(ifStmt != nullptr);
        auto* cond = dynamic_cast<BinaryExpr*>(ifStmt->condition.get());
        REQUIRE(cond != nullptr);
        REQUIRE(cond->op.type == TokenType::EQUAL_EQUAL);
    }
}

TEST_CASE("Parser handles while statements", "[parser][core]") {
    SECTION("Parse basic while loop") {
        auto stmts = parseSource("while (i < 10) { i = i + 1; }");
        REQUIRE(stmts.size() == 1);
        auto* whileStmt = dynamic_cast<WhileStmt*>(stmts[0].get());
        REQUIRE(whileStmt != nullptr);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->body != nullptr);
    }

    SECTION("Parse while with break and continue") {
        auto stmts = parseSource("while (true) { if (done) { break; } continue; }");
        REQUIRE(stmts.size() == 1);
        auto* whileStmt = dynamic_cast<WhileStmt*>(stmts[0].get());
        REQUIRE(whileStmt != nullptr);
        auto* body = dynamic_cast<BlockStmt*>(whileStmt->body.get());
        REQUIRE(body != nullptr);
        REQUIRE(body->statements.size() == 2);
    }
}

TEST_CASE("Parser handles for statements", "[parser][core]") {
    SECTION("Parse basic for loop desugared to while") {
        auto stmts = parseSource("for (var i = 0; i < 10; i = i + 1) { }");
        REQUIRE(stmts.size() == 1);
        // for loops desugar to a block containing a while loop
        auto* block = dynamic_cast<BlockStmt*>(stmts[0].get());
        REQUIRE(block != nullptr);
        REQUIRE(block->statements.size() == 2);
        // First: initializer (VarStmt)
        REQUIRE(dynamic_cast<VarStmt*>(block->statements[0].get()) != nullptr);
        // Second: while loop
        REQUIRE(dynamic_cast<WhileStmt*>(block->statements[1].get()) != nullptr);
    }

    SECTION("Parse for loop without initializer") {
        auto stmts = parseSource("for (; i < 5; i = i + 1) { }");
        REQUIRE(stmts.size() == 1);
        // No initializer → desugars directly to a while
        auto* whileStmt = dynamic_cast<WhileStmt*>(stmts[0].get());
        REQUIRE(whileStmt != nullptr);
    }
}

TEST_CASE("Parser handles return statements", "[parser][core]") {
    SECTION("Parse return without value") {
        auto stmts = parseSource("fn f() { return; }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->body.size() == 1);
        auto* ret = dynamic_cast<ReturnStmt*>(fn->body[0].get());
        REQUIRE(ret != nullptr);
        REQUIRE(ret->value == nullptr);
    }

    SECTION("Parse return with expression") {
        auto stmts = parseSource("fn f() { return 42; }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        auto* ret = dynamic_cast<ReturnStmt*>(fn->body[0].get());
        REQUIRE(ret != nullptr);
        REQUIRE(ret->value != nullptr);
        auto* lit = dynamic_cast<LiteralExpr*>(ret->value.get());
        REQUIRE(lit != nullptr);
        REQUIRE(std::get<double>(lit->value) == 42.0);
    }

    SECTION("Parse return with complex expression") {
        auto stmts = parseSource("fn f(a, b) { return a + b * 2; }");
        REQUIRE(stmts.size() == 1);
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        auto* ret = dynamic_cast<ReturnStmt*>(fn->body[0].get());
        REQUIRE(ret != nullptr);
        REQUIRE(ret->value != nullptr);
        auto* binExpr = dynamic_cast<BinaryExpr*>(ret->value.get());
        REQUIRE(binExpr != nullptr);
        REQUIRE(binExpr->op.type == TokenType::PLUS);
    }
}

TEST_CASE("Parser handles expressions", "[parser][core]") {
    SECTION("Parse arithmetic binary expression") {
        auto stmts = parseSource("var x = 1 + 2 * 3;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        // + has lower precedence: (1 + (2 * 3))
        auto* add = dynamic_cast<BinaryExpr*>(var->initializer.get());
        REQUIRE(add != nullptr);
        REQUIRE(add->op.type == TokenType::PLUS);
        auto* mul = dynamic_cast<BinaryExpr*>(add->right.get());
        REQUIRE(mul != nullptr);
        REQUIRE(mul->op.type == TokenType::STAR);
    }

    SECTION("Parse comparison expressions") {
        auto stmts = parseSource("var r = a >= b;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        auto* cmp = dynamic_cast<BinaryExpr*>(var->initializer.get());
        REQUIRE(cmp != nullptr);
        REQUIRE(cmp->op.type == TokenType::GREATER_EQUAL);
    }

    SECTION("Parse logical and/or expressions") {
        auto stmts = parseSource("var r = a and b or c;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        // 'or' has lower precedence: ((a and b) or c)
        auto* orExpr = dynamic_cast<BinaryExpr*>(var->initializer.get());
        REQUIRE(orExpr != nullptr);
        REQUIRE(orExpr->op.type == TokenType::OR);
        auto* andExpr = dynamic_cast<BinaryExpr*>(orExpr->left.get());
        REQUIRE(andExpr != nullptr);
        REQUIRE(andExpr->op.type == TokenType::AND);
    }

    SECTION("Parse unary negation") {
        auto stmts = parseSource("var x = -5;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        auto* unary = dynamic_cast<UnaryExpr*>(var->initializer.get());
        REQUIRE(unary != nullptr);
        REQUIRE(unary->op.type == TokenType::MINUS);
    }

    SECTION("Parse unary logical not") {
        auto stmts = parseSource("var x = !done;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        auto* unary = dynamic_cast<UnaryExpr*>(var->initializer.get());
        REQUIRE(unary != nullptr);
        REQUIRE(unary->op.type == TokenType::BANG);
    }

    SECTION("Parse grouping overrides precedence") {
        auto stmts = parseSource("var x = (1 + 2) * 3;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        // * at top level: ((1+2) * 3)
        auto* mul = dynamic_cast<BinaryExpr*>(var->initializer.get());
        REQUIRE(mul != nullptr);
        REQUIRE(mul->op.type == TokenType::STAR);
        auto* grp = dynamic_cast<GroupingExpr*>(mul->left.get());
        REQUIRE(grp != nullptr);
    }

    SECTION("Parse equality expressions") {
        auto stmts = parseSource("var eq = x != y;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        auto* expr = dynamic_cast<BinaryExpr*>(var->initializer.get());
        REQUIRE(expr != nullptr);
        REQUIRE(expr->op.type == TokenType::BANG_EQUAL);
    }

    SECTION("Parse assignment expression") {
        auto stmts = parseSource("x = 10;");
        REQUIRE(stmts.size() == 1);
        auto* exprStmt = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(exprStmt != nullptr);
        auto* assign = dynamic_cast<AssignExpr*>(exprStmt->expr.get());
        REQUIRE(assign != nullptr);
        REQUIRE(assign->name == "x");
    }

    SECTION("Parse function call expression") {
        auto stmts = parseSource("var r = add(1, 2);");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        auto* call = dynamic_cast<CallExpr*>(var->initializer.get());
        REQUIRE(call != nullptr);
        REQUIRE(call->args.size() == 2);
    }

    SECTION("Parse literal values: numbers, strings, booleans, nil") {
        auto stmts = parseSource(R"(
            var n = 3.14;
            var s = "hello";
            var b = true;
            var z = nil;
        )");
        REQUIRE(stmts.size() == 4);

        auto* nVar = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(nVar != nullptr);
        auto* numLit = dynamic_cast<LiteralExpr*>(nVar->initializer.get());
        REQUIRE(numLit != nullptr);
        REQUIRE(std::get<double>(numLit->value) == Approx(3.14));

        auto* sVar = dynamic_cast<VarStmt*>(stmts[1].get());
        REQUIRE(sVar != nullptr);
        auto* strLit = dynamic_cast<LiteralExpr*>(sVar->initializer.get());
        REQUIRE(strLit != nullptr);
        REQUIRE(std::get<std::string>(strLit->value) == "hello");

        auto* bVar = dynamic_cast<VarStmt*>(stmts[2].get());
        REQUIRE(bVar != nullptr);
        auto* boolLit = dynamic_cast<LiteralExpr*>(bVar->initializer.get());
        REQUIRE(boolLit != nullptr);
        REQUIRE(std::get<bool>(boolLit->value) == true);

        auto* zVar = dynamic_cast<VarStmt*>(stmts[3].get());
        REQUIRE(zVar != nullptr);
        auto* nilLit = dynamic_cast<LiteralExpr*>(zVar->initializer.get());
        REQUIRE(nilLit != nullptr);
        REQUIRE(std::holds_alternative<Nil>(nilLit->value));
    }
}

TEST_CASE("Parser handles variable declarations", "[parser][core]") {
    SECTION("Parse var with initializer") {
        auto stmts = parseSource("var x = 42;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        REQUIRE(var->name == "x");
        REQUIRE(var->initializer != nullptr);
    }

    SECTION("Parse var without initializer") {
        auto stmts = parseSource("var x;");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        REQUIRE(var->name == "x");
        REQUIRE(var->initializer == nullptr);
    }

    SECTION("Parse array destructuring") {
        auto stmts = parseSource("var [a, b] = [1, 2];");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        REQUIRE(var->pattern != nullptr);
        REQUIRE(var->initializer != nullptr);
    }

    SECTION("Parse map destructuring") {
        auto stmts = parseSource(R"(var {x, y} = point;)");
        REQUIRE(stmts.size() == 1);
        auto* var = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(var != nullptr);
        REQUIRE(var->pattern != nullptr);
        REQUIRE(var->initializer != nullptr);
    }
}

TEST_CASE("Parser handles syntax error recovery", "[parser][core]") {
    SECTION("Recovers after a bad expression and continues parsing") {
        // Missing closing paren — parser should recover and still parse the next statement
        const std::string src = "var x = (1 + ; var y = 2;";
        DiagnosticEngine diags(src);
        Lexer lexer(src);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), src, &diags);
        auto stmts = parser.parse();
        // An error must have been reported
        REQUIRE(diags.hasErrors());
        // After recovery, the second var declaration should still be produced
        bool foundY = false;
        for (auto& s : stmts) {
            if (auto* v = dynamic_cast<VarStmt*>(s.get())) {
                if (v->name == "y") foundY = true;
            }
        }
        REQUIRE(foundY);
    }

    SECTION("Recovers after missing closing brace in if body") {
        // Malformed if — parser synchronizes and picks up next declaration
        const std::string src = "if (true) { var a = 1; fn f() { }";
        DiagnosticEngine diags(src);
        Lexer lexer(src);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), src, &diags);
        // Should not throw; may produce partial/empty results but must not crash
        REQUIRE_NOTHROW(parser.parse());
        // The unclosed block is a parse error
        REQUIRE(diags.hasErrors());
    }

    SECTION("Reports error for missing expression after operator") {
        const std::string src = "var x = 1 +;";
        DiagnosticEngine diags(src);
        Lexer lexer(src);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), src, &diags);
        // parse() catches and recovers; result may be empty or contain nil stmts
        REQUIRE_NOTHROW(parser.parse());
        // The missing right-hand operand must be flagged
        REQUIRE(diags.hasErrors());
    }
}

TEST_CASE("Parser integrates lexer and produces valid AST", "[parser][core]") {
    SECTION("Parse a representative program") {
        auto stmts = parseSource(R"(
            fn fibonacci(n) {
                if (n <= 1) {
                    return n;
                }
                return fibonacci(n - 1) + fibonacci(n - 2);
            }
            var result = fibonacci(10);
        )");

        REQUIRE(stmts.size() == 2);

        // First: function declaration
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->name == "fibonacci");
        REQUIRE(fn->params.size() == 1);
        REQUIRE(fn->params[0] == "n");
        REQUIRE(fn->body.size() == 2);

        // Second: variable declaration
        auto* var = dynamic_cast<VarStmt*>(stmts[1].get());
        REQUIRE(var != nullptr);
        REQUIRE(var->name == "result");
        auto* call = dynamic_cast<CallExpr*>(var->initializer.get());
        REQUIRE(call != nullptr);
        REQUIRE(call->args.size() == 1);
    }

    SECTION("Parse a while loop program") {
        auto stmts = parseSource(R"(
            var sum = 0;
            var i = 1;
            while (i <= 100) {
                sum = sum + i;
                i = i + 1;
            }
        )");

        REQUIRE(stmts.size() == 3);
        REQUIRE(dynamic_cast<VarStmt*>(stmts[0].get()) != nullptr);
        REQUIRE(dynamic_cast<VarStmt*>(stmts[1].get()) != nullptr);
        auto* whileStmt = dynamic_cast<WhileStmt*>(stmts[2].get());
        REQUIRE(whileStmt != nullptr);
        auto* body = dynamic_cast<BlockStmt*>(whileStmt->body.get());
        REQUIRE(body != nullptr);
        REQUIRE(body->statements.size() == 2);
    }
}

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

    SECTION("Parse function type with initializer") {
        Lexer lexer("var callback: fn(Number) -> Number = fn(x) { return x * 2; };");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Function);
        REQUIRE(varStmt->typeAnnotation->paramTypes.size() == 1);
        REQUIRE(varStmt->typeAnnotation->paramTypes[0]->kind == TypeAnnotation::Kind::Number);
        REQUIRE(varStmt->typeAnnotation->valueType->kind == TypeAnnotation::Kind::Number);
        REQUIRE(varStmt->initializer != nullptr);
        auto* funcExpr = dynamic_cast<FunctionExpr*>(varStmt->initializer.get());
        REQUIRE(funcExpr != nullptr);
    }
}

TEST_CASE("Parser handles edge case type annotations", "[parser][types]") {
    SECTION("Parse variable with Nil type") {
        Lexer lexer("var empty: Nil = nil;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Nil);
    }

    SECTION("Parse variable with Any type") {
        Lexer lexer("var dynamic: Any = 42;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Any);
    }

    SECTION("Parse nested complex type") {
        Lexer lexer("var data: Array<Map<String,Number>> = [];");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->typeAnnotation != nullptr);
        REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Array);
        REQUIRE(varStmt->typeAnnotation->elementType != nullptr);
        REQUIRE(varStmt->typeAnnotation->elementType->kind == TypeAnnotation::Kind::Map);
        REQUIRE(varStmt->typeAnnotation->elementType->keyType->kind == TypeAnnotation::Kind::String);
        REQUIRE(varStmt->typeAnnotation->elementType->valueType->kind == TypeAnnotation::Kind::Number);
    }

    SECTION("Parse function with Void return and no parameters") {
        Lexer lexer("fn doSomething(): Void { print(42); }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* funcStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(funcStmt != nullptr);
        REQUIRE(funcStmt->returnType != nullptr);
        REQUIRE(funcStmt->returnType->kind == TypeAnnotation::Kind::Void);
        REQUIRE(funcStmt->params.size() == 0);
    }
}

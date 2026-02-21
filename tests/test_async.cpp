#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "ast/stmt.hpp"
#include "ast/expr.hpp"
#include "interp/interpreter.hpp"

using namespace izi;

// ── Lexer tests ──────────────────────────────────────────────────────────────

TEST_CASE("Lexer tokenizes async and await keywords", "[lexer][async]") {
    SECTION("Tokenizes 'async' as ASYNC token") {
        Lexer lexer("async");
        auto tokens = lexer.scanTokens();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::ASYNC);
        REQUIRE(tokens[0].lexeme == "async");
    }

    SECTION("Tokenizes 'await' as AWAIT token") {
        Lexer lexer("await");
        auto tokens = lexer.scanTokens();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::AWAIT);
        REQUIRE(tokens[0].lexeme == "await");
    }

    SECTION("Tokenizes 'async fn' correctly") {
        Lexer lexer("async fn");
        auto tokens = lexer.scanTokens();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].type == TokenType::ASYNC);
        REQUIRE(tokens[1].type == TokenType::FN);
    }

    SECTION("async and await remain identifiers when not keywords") {
        // 'asyncVar' should be an IDENTIFIER, not ASYNC
        Lexer lexer("asyncVar awaitResult");
        auto tokens = lexer.scanTokens();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
    }
}

// ── Parser tests ─────────────────────────────────────────────────────────────

TEST_CASE("Parser handles async function declarations", "[parser][async]") {
    SECTION("Parse async fn with isAsync=true") {
        Lexer lexer("async fn greet() { return 42; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* fnStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fnStmt != nullptr);
        REQUIRE(fnStmt->name == "greet");
        REQUIRE(fnStmt->isAsync == true);
    }

    SECTION("Regular fn has isAsync=false") {
        Lexer lexer("fn greet() { return 42; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* fnStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fnStmt != nullptr);
        REQUIRE(fnStmt->isAsync == false);
    }

    SECTION("Parse async fn with parameters") {
        Lexer lexer("async fn add(a, b) { return a + b; }");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* fnStmt = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fnStmt != nullptr);
        REQUIRE(fnStmt->isAsync == true);
        REQUIRE(fnStmt->params.size() == 2);
    }
}

TEST_CASE("Parser handles await expressions", "[parser][async]") {
    SECTION("Parse await as unary expression in var declaration") {
        Lexer lexer("var result = await someTask;");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        REQUIRE(varStmt->name == "result");
        auto* awaitExpr = dynamic_cast<AwaitExpr*>(varStmt->initializer.get());
        REQUIRE(awaitExpr != nullptr);
    }

    SECTION("Parse await function call") {
        Lexer lexer("var data = await fetchData();");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        auto* awaitExpr = dynamic_cast<AwaitExpr*>(varStmt->initializer.get());
        REQUIRE(awaitExpr != nullptr);
        auto* callExpr = dynamic_cast<CallExpr*>(awaitExpr->value.get());
        REQUIRE(callExpr != nullptr);
    }
}

TEST_CASE("Parser handles async function expressions", "[parser][async]") {
    SECTION("Parse async fn expression") {
        Lexer lexer("var f = async fn() { return 1; };");
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        REQUIRE(stmts.size() == 1);
        auto* varStmt = dynamic_cast<VarStmt*>(stmts[0].get());
        REQUIRE(varStmt != nullptr);
        auto* fnExpr = dynamic_cast<FunctionExpr*>(varStmt->initializer.get());
        REQUIRE(fnExpr != nullptr);
        REQUIRE(fnExpr->isAsync == true);
    }
}

// ── Interpreter tests ─────────────────────────────────────────────────────────

TEST_CASE("Interpreter: async fn returns a Task", "[interpreter][async]") {
    SECTION("Calling async fn returns a pending Task") {
        std::string code = R"(
            async fn getValue() {
                return 42;
            }
            var t = getValue();
        )";
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        Interpreter interp;
        interp.interpret(stmts);

        Value t = interp.getGlobals()->get("t");
        REQUIRE(std::holds_alternative<std::shared_ptr<Task>>(t));
        auto task = std::get<std::shared_ptr<Task>>(t);
        REQUIRE(task->state == Task::State::Pending);
    }

    SECTION("await on async fn result runs the task and returns value") {
        std::string code = R"(
            async fn getValue() {
                return 99;
            }
            var result = await getValue();
        )";
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        Interpreter interp;
        interp.interpret(stmts);

        Value result = interp.getGlobals()->get("result");
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 99.0);
    }

    SECTION("await on non-task value returns value unchanged") {
        std::string code = R"(
            var result = await 42;
        )";
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        Interpreter interp;
        interp.interpret(stmts);

        Value result = interp.getGlobals()->get("result");
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 42.0);
    }

    SECTION("async fn with parameters works correctly when awaited") {
        std::string code = R"(
            async fn add(a, b) {
                return a + b;
            }
            var result = await add(3, 4);
        )";
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        Interpreter interp;
        interp.interpret(stmts);

        Value result = interp.getGlobals()->get("result");
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 7.0);
    }

    SECTION("async fn calling another async fn with await") {
        std::string code = R"(
            async fn inner() {
                return 10;
            }
            async fn outer() {
                var x = await inner();
                return x * 2;
            }
            var result = await outer();
        )";
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        Interpreter interp;
        interp.interpret(stmts);

        Value result = interp.getGlobals()->get("result");
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 20.0);
    }

    SECTION("await on regular function call returns value directly") {
        std::string code = R"(
            fn regularAdd(a, b) {
                return a + b;
            }
            var result = await regularAdd(5, 6);
        )";
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        Interpreter interp;
        interp.interpret(stmts);

        Value result = interp.getGlobals()->get("result");
        REQUIRE(std::holds_alternative<double>(result));
        REQUIRE(std::get<double>(result) == 11.0);
    }
}

TEST_CASE("Interpreter: async fn interacts with spawn/await native functions", "[interpreter][async]") {
    SECTION("spawn still creates a pending task") {
        std::string code = R"(
            var t = spawn(fn() { return 5; });
        )";
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto stmts = parser.parse();

        Interpreter interp;
        interp.interpret(stmts);

        Value t = interp.getGlobals()->get("t");
        REQUIRE(std::holds_alternative<std::shared_ptr<Task>>(t));
    }
}

#include "catch.hpp"
#include "ast/ast_printer.hpp"
#include "ast/expr.hpp"
#include "ast/stmt.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"

using namespace izi;

// ---------------------------------------------------------------------------
// Helper: lex + parse source string, then pretty-print via AstPrinter
// ---------------------------------------------------------------------------
static std::vector<StmtPtr> parseSource(const std::string& src) {
    Lexer lexer(src);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens));
    return parser.parse();
}

static std::string printSource(const std::string& src) {
    auto stmts = parseSource(src);
    AstPrinter printer;
    return printer.print(stmts);
}

// ===========================================================================
// Tests for AST node construction
// ===========================================================================

TEST_CASE("AstPrinter: literal expressions", "[ast_printer]") {
    SECTION("number literal") {
        auto stmts = parseSource("42;");
        REQUIRE(stmts.size() == 1);
        auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(es != nullptr);
        auto* lit = dynamic_cast<LiteralExpr*>(es->expr.get());
        REQUIRE(lit != nullptr);
        REQUIRE(std::holds_alternative<double>(lit->value));
        REQUIRE(std::get<double>(lit->value) == 42.0);
        AstPrinter printer;
        REQUIRE(printer.printExpr(*es->expr) == "42");
    }

    SECTION("string literal") {
        auto stmts = parseSource("\"hello\";");
        auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(es != nullptr);
        AstPrinter printer;
        REQUIRE(printer.printExpr(*es->expr) == "\"hello\"");
    }

    SECTION("true literal") {
        auto stmts = parseSource("true;");
        auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
        AstPrinter printer;
        REQUIRE(printer.printExpr(*es->expr) == "true");
    }

    SECTION("false literal") {
        auto stmts = parseSource("false;");
        auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
        AstPrinter printer;
        REQUIRE(printer.printExpr(*es->expr) == "false");
    }

    SECTION("nil literal") {
        auto stmts = parseSource("nil;");
        auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
        AstPrinter printer;
        REQUIRE(printer.printExpr(*es->expr) == "nil");
    }
}

TEST_CASE("AstPrinter: variable and assignment expressions", "[ast_printer]") {
    SECTION("variable reference") {
        auto stmts = parseSource("x;");
        auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(es != nullptr);
        auto* var = dynamic_cast<VariableExpr*>(es->expr.get());
        REQUIRE(var != nullptr);
        REQUIRE(var->name == "x");
        AstPrinter printer;
        REQUIRE(printer.printExpr(*es->expr) == "x");
    }

    SECTION("assignment expression") {
        auto stmts = parseSource("x = 5;");
        auto* es = dynamic_cast<ExprStmt*>(stmts[0].get());
        REQUIRE(es != nullptr);
        AstPrinter printer;
        REQUIRE(printer.printExpr(*es->expr) == "(= x 5)");
    }
}

TEST_CASE("AstPrinter: binary expressions", "[ast_printer]") {
    SECTION("addition") {
        REQUIRE(printSource("1 + 2;") == "(+ 1 2)\n");
    }

    SECTION("subtraction") {
        REQUIRE(printSource("a - b;") == "(- a b)\n");
    }

    SECTION("multiplication") {
        REQUIRE(printSource("3 * 4;") == "(* 3 4)\n");
    }

    SECTION("division") {
        REQUIRE(printSource("10 / 2;") == "(/ 10 2)\n");
    }

    SECTION("equality") {
        REQUIRE(printSource("a == b;") == "(== a b)\n");
    }

    SECTION("inequality") {
        REQUIRE(printSource("a != b;") == "(!= a b)\n");
    }

    SECTION("less than") {
        REQUIRE(printSource("a < b;") == "(< a b)\n");
    }

    SECTION("greater than") {
        REQUIRE(printSource("a > b;") == "(> a b)\n");
    }

    SECTION("nested binary") {
        REQUIRE(printSource("1 + 2 * 3;") == "(+ 1 (* 2 3))\n");
    }
}

TEST_CASE("AstPrinter: unary expressions", "[ast_printer]") {
    SECTION("negation") {
        REQUIRE(printSource("-x;") == "(- x)\n");
    }

    SECTION("logical not") {
        REQUIRE(printSource("!x;") == "(! x)\n");
    }
}

TEST_CASE("AstPrinter: grouping expression", "[ast_printer]") {
    SECTION("grouped addition") {
        REQUIRE(printSource("(1 + 2);") == "(group (+ 1 2))\n");
    }
}

TEST_CASE("AstPrinter: call expression", "[ast_printer]") {
    SECTION("no-arg call") {
        REQUIRE(printSource("foo();") == "(call foo)\n");
    }

    SECTION("call with arguments") {
        REQUIRE(printSource("add(1, 2);") == "(call add 1 2)\n");
    }
}

TEST_CASE("AstPrinter: ternary/conditional expression", "[ast_printer]") {
    SECTION("ternary") {
        REQUIRE(printSource("x ? 1 : 2;") == "(?: x 1 2)\n");
    }
}

TEST_CASE("AstPrinter: array expression", "[ast_printer]") {
    SECTION("empty array") {
        REQUIRE(printSource("[];") == "(array)\n");
    }

    SECTION("array with elements") {
        REQUIRE(printSource("[1, 2, 3];") == "(array 1 2 3)\n");
    }
}

TEST_CASE("AstPrinter: index expression", "[ast_printer]") {
    SECTION("index access") {
        REQUIRE(printSource("arr[0];") == "(index arr 0)\n");
    }
}

TEST_CASE("AstPrinter: property access", "[ast_printer]") {
    SECTION("property get") {
        REQUIRE(printSource("obj.name;") == "(. obj name)\n");
    }
}

TEST_CASE("AstPrinter: this and super expressions", "[ast_printer]") {
    SECTION("this expression") {
        // 'this' is valid inside a class method
        auto stmts = parseSource("class C { fn f() { this; } }");
        AstPrinter printer;
        std::string out = printer.print(stmts);
        REQUIRE(out.find("this") != std::string::npos);
    }
}

// ===========================================================================
// Tests for AST statement construction and pretty-print
// ===========================================================================

TEST_CASE("AstPrinter: variable declaration statement", "[ast_printer]") {
    SECTION("simple var") {
        REQUIRE(printSource("var x = 10;") == "(var x 10)\n");
    }

    SECTION("var with expression") {
        REQUIRE(printSource("var y = 1 + 2;") == "(var y (+ 1 2))\n");
    }
}

TEST_CASE("AstPrinter: block statement", "[ast_printer]") {
    SECTION("empty block") {
        auto stmts = parseSource("fn f() { }");
        auto* fn = dynamic_cast<FunctionStmt*>(stmts[0].get());
        REQUIRE(fn != nullptr);
        REQUIRE(fn->body.empty());
    }

    SECTION("block with statements") {
        REQUIRE(printSource("fn f() { var x = 1; var y = 2; }") ==
                "(fn f () (var x 1) (var y 2))\n");
    }
}

TEST_CASE("AstPrinter: if statement", "[ast_printer]") {
    SECTION("if without else") {
        REQUIRE(printSource("if (x) { }") == "(if x (block))\n");
    }

    SECTION("if with else") {
        REQUIRE(printSource("if (x) { } else { }") == "(if x (block) (block))\n");
    }
}

TEST_CASE("AstPrinter: while statement", "[ast_printer]") {
    SECTION("while loop") {
        REQUIRE(printSource("while (x) { }") == "(while x (block))\n");
    }
}

TEST_CASE("AstPrinter: return statement", "[ast_printer]") {
    SECTION("return with value") {
        REQUIRE(printSource("fn f() { return 42; }") == "(fn f () (return 42))\n");
    }

    SECTION("return without value") {
        REQUIRE(printSource("fn f() { return; }") == "(fn f () (return))\n");
    }
}

TEST_CASE("AstPrinter: function declaration", "[ast_printer]") {
    SECTION("no params, empty body") {
        REQUIRE(printSource("fn greet() { }") == "(fn greet ())\n");
    }

    SECTION("with params") {
        REQUIRE(printSource("fn add(a, b) { return a + b; }") ==
                "(fn add (a b) (return (+ a b)))\n");
    }

    SECTION("anonymous function expression") {
        REQUIRE(printSource("var f = fn(x) { return x; };") ==
                "(var f (fn (x) (return x)))\n");
    }
}

TEST_CASE("AstPrinter: break and continue", "[ast_printer]") {
    SECTION("break") {
        REQUIRE(printSource("while (true) { break; }") == "(while true (block break))\n");
    }

    SECTION("continue") {
        REQUIRE(printSource("while (true) { continue; }") == "(while true (block continue))\n");
    }
}

TEST_CASE("AstPrinter: throw statement", "[ast_printer]") {
    SECTION("throw string") {
        REQUIRE(printSource("throw \"error\";") == "(throw \"error\")\n");
    }
}

TEST_CASE("AstPrinter: try-catch statement", "[ast_printer]") {
    SECTION("try-catch") {
        std::string out = printSource("try { } catch(e) { }");
        REQUIRE(out.find("(try") != std::string::npos);
        REQUIRE(out.find("(catch e") != std::string::npos);
    }

    SECTION("try-catch-finally") {
        std::string out = printSource("try { } catch(e) { } finally { }");
        REQUIRE(out.find("(try") != std::string::npos);
        REQUIRE(out.find("(catch e") != std::string::npos);
        REQUIRE(out.find("(finally") != std::string::npos);
    }
}

TEST_CASE("AstPrinter: import statement", "[ast_printer]") {
    SECTION("simple import") {
        REQUIRE(printSource("import \"mod\";") == "(import \"mod\")\n");
    }

    SECTION("named import") {
        std::string out = printSource("import { foo, bar } from \"mod\";");
        REQUIRE(out.find("(import") != std::string::npos);
        REQUIRE(out.find("\"mod\"") != std::string::npos);
        REQUIRE(out.find("foo") != std::string::npos);
        REQUIRE(out.find("bar") != std::string::npos);
    }
}

TEST_CASE("AstPrinter: class declaration", "[ast_printer]") {
    SECTION("simple class") {
        std::string out = printSource("class Point { }");
        REQUIRE(out.find("(class Point") != std::string::npos);
    }

    SECTION("class with inheritance") {
        std::string out = printSource("class Dog extends Animal { }");
        REQUIRE(out.find("(class Dog") != std::string::npos);
        REQUIRE(out.find("(extends Animal)") != std::string::npos);
    }

    SECTION("class with method") {
        std::string out = printSource("class C { fn hello() { return 1; } }");
        REQUIRE(out.find("(class C") != std::string::npos);
        REQUIRE(out.find("(fn hello") != std::string::npos);
    }
}

TEST_CASE("AstPrinter: multi-statement program", "[ast_printer]") {
    SECTION("multiple top-level statements") {
        std::string out = printSource("var x = 1;\nvar y = 2;");
        REQUIRE(out == "(var x 1)\n(var y 2)\n");
    }
}

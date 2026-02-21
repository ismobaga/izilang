#include "catch.hpp"
#include "compile/formatter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"

using namespace izi;

// Helper: parse source code into an AST
static std::vector<StmtPtr> parse(const std::string& code) {
    Lexer lexer(code);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), code);
    return parser.parse();
}

// Helper: format source code
static std::string fmt(const std::string& code) {
    auto program = parse(code);
    Formatter formatter;
    return formatter.format(program);
}

// ── Variable declarations ─────────────────────────────────────────────────────

TEST_CASE("Formatter: variable declarations", "[formatter]") {
    SECTION("simple var") {
        REQUIRE(fmt("var x=1;") == "var x = 1;\n");
    }

    SECTION("string literal") {
        REQUIRE(fmt("var s=\"hello\";") == "var s = \"hello\";\n");
    }

    SECTION("nil literal") {
        REQUIRE(fmt("var n=nil;") == "var n = nil;\n");
    }

    SECTION("boolean literals") {
        REQUIRE(fmt("var a=true;var b=false;") == "var a = true;\nvar b = false;\n");
    }
}

// ── Binary expressions ────────────────────────────────────────────────────────

TEST_CASE("Formatter: binary expressions", "[formatter]") {
    SECTION("addition") {
        REQUIRE(fmt("var x=1+2;") == "var x = 1 + 2;\n");
    }

    SECTION("comparison") {
        REQUIRE(fmt("var x=a==b;") == "var x = a == b;\n");
    }

    SECTION("logical") {
        REQUIRE(fmt("var x=a and b;") == "var x = a and b;\n");
    }
}

// ── Unary expressions ─────────────────────────────────────────────────────────

TEST_CASE("Formatter: unary expressions", "[formatter]") {
    SECTION("negation") {
        REQUIRE(fmt("var x=-1;") == "var x = -1;\n");
    }

    SECTION("logical not") {
        REQUIRE(fmt("var x=!true;") == "var x = !true;\n");
    }
}

// ── Grouping ──────────────────────────────────────────────────────────────────

TEST_CASE("Formatter: grouping", "[formatter]") {
    REQUIRE(fmt("var x=(1+2)*3;") == "var x = (1 + 2) * 3;\n");
}

// ── Ternary operator ──────────────────────────────────────────────────────────

TEST_CASE("Formatter: ternary operator", "[formatter]") {
    REQUIRE(fmt("var x=a?1:2;") == "var x = a ? 1 : 2;\n");
}

// ── Functions ─────────────────────────────────────────────────────────────────

TEST_CASE("Formatter: function declarations", "[formatter]") {
    SECTION("basic function") {
        std::string input = "fn add(a,b){return a+b;}";
        std::string expected =
            "fn add(a, b) {\n"
            "    return a + b;\n"
            "}\n";
        REQUIRE(fmt(input) == expected);
    }

    SECTION("function with no params") {
        std::string input = "fn greet(){print(\"hi\");}";
        std::string expected =
            "fn greet() {\n"
            "    print(\"hi\");\n"
            "}\n";
        REQUIRE(fmt(input) == expected);
    }

    SECTION("blank line between two functions") {
        std::string input = "fn f(){} fn g(){}";
        std::string result = fmt(input);
        REQUIRE(result.find("\n\n") != std::string::npos);
    }
}

// ── Control flow ──────────────────────────────────────────────────────────────

TEST_CASE("Formatter: if statement", "[formatter]") {
    SECTION("if without else") {
        std::string input = "if(x>0){print(x);}";
        std::string expected =
            "if (x > 0) {\n"
            "    print(x);\n"
            "}\n";
        REQUIRE(fmt(input) == expected);
    }

    SECTION("if with else") {
        std::string input = "if(x>0){print(1);}else{print(2);}";
        std::string expected =
            "if (x > 0) {\n"
            "    print(1);\n"
            "} else {\n"
            "    print(2);\n"
            "}\n";
        REQUIRE(fmt(input) == expected);
    }
}

TEST_CASE("Formatter: while loop", "[formatter]") {
    std::string input = "while(i<10){i=i+1;}";
    std::string expected =
        "while (i < 10) {\n"
        "    i = i + 1;\n"
        "}\n";
    REQUIRE(fmt(input) == expected);
}

// ── Collections ───────────────────────────────────────────────────────────────

TEST_CASE("Formatter: array literals", "[formatter]") {
    REQUIRE(fmt("var a=[1,2,3];") == "var a = [1, 2, 3];\n");
}

TEST_CASE("Formatter: map literals", "[formatter]") {
    REQUIRE(fmt("var m={\"a\":1,\"b\":2};") == "var m = {\"a\": 1, \"b\": 2};\n");
}

TEST_CASE("Formatter: index expression", "[formatter]") {
    REQUIRE(fmt("var x=arr[0];") == "var x = arr[0];\n");
}

// ── Imports ───────────────────────────────────────────────────────────────────

TEST_CASE("Formatter: import statements", "[formatter]") {
    SECTION("named imports") {
        REQUIRE(fmt("import{sqrt,pow}from \"math\";") ==
                "import { sqrt, pow } from \"math\";\n");
    }

    SECTION("wildcard import") {
        REQUIRE(fmt("import * as math from \"math\";") ==
                "import * as math from \"math\";\n");
    }

    SECTION("simple import") {
        REQUIRE(fmt("import \"utils\";") == "import \"utils\";\n");
    }
}

// ── Try/catch ─────────────────────────────────────────────────────────────────

TEST_CASE("Formatter: try/catch/finally", "[formatter]") {
    std::string input = "try{var x=1;}catch(e){print(e);}finally{print(\"done\");}";
    std::string result = fmt(input);
    REQUIRE(result.find("try {") != std::string::npos);
    REQUIRE(result.find("} catch (e) {") != std::string::npos);
    REQUIRE(result.find("} finally {") != std::string::npos);
}

// ── Classes ───────────────────────────────────────────────────────────────────

TEST_CASE("Formatter: class declarations", "[formatter]") {
    SECTION("basic class") {
        std::string input = "class Dog{fn speak(){print(\"woof\");}}";
        std::string result = fmt(input);
        REQUIRE(result.find("class Dog {") != std::string::npos);
        REQUIRE(result.find("fn speak() {") != std::string::npos);
    }

    SECTION("class with inheritance") {
        std::string input = "class Dog extends Animal{fn speak(){}}";
        std::string result = fmt(input);
        REQUIRE(result.find("class Dog extends Animal {") != std::string::npos);
    }
}

// ── Break / continue ──────────────────────────────────────────────────────────

TEST_CASE("Formatter: break and continue", "[formatter]") {
    SECTION("break") {
        std::string input = "while(true){break;}";
        std::string result = fmt(input);
        REQUIRE(result.find("    break;") != std::string::npos);
    }

    SECTION("continue") {
        std::string input = "while(true){continue;}";
        std::string result = fmt(input);
        REQUIRE(result.find("    continue;") != std::string::npos);
    }
}

// ── Idempotency ───────────────────────────────────────────────────────────────

TEST_CASE("Formatter: idempotent (formatting twice gives same result)", "[formatter]") {
    std::string input = "fn add(a,b){return a+b;}var x=add(1,2);print(x);";
    std::string first = fmt(input);
    std::string second = fmt(first);
    REQUIRE(first == second);
}

// ── FormatterConfig ───────────────────────────────────────────────────────────

TEST_CASE("FormatterConfig: default values", "[formatter][config]") {
    FormatterConfig cfg;
    REQUIRE(cfg.indentSize == 4);
    REQUIRE(cfg.maxLineLength == 100);
}

TEST_CASE("FormatterConfig: custom indent size", "[formatter][config]") {
    FormatterConfig cfg;
    cfg.indentSize = 2;

    auto program = parse("fn add(a,b){return a+b;}");
    Formatter formatter(cfg);
    std::string result = formatter.format(program);

    REQUIRE(result.find("  return") != std::string::npos);
    REQUIRE(result.find("    return") == std::string::npos);
}

TEST_CASE("FormatterConfig: load from non-existent file returns defaults", "[formatter][config]") {
    FormatterConfig cfg = FormatterConfig::load("/nonexistent/.izifmt.toml");
    REQUIRE(cfg.indentSize == 4);
    REQUIRE(cfg.maxLineLength == 100);
}


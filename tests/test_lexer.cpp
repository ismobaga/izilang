#include "catch.hpp"
#include "parse/lexer.hpp"
#include "common/token.hpp"

using namespace izi;

TEST_CASE("Lexer tokenizes single character tokens", "[lexer]") {
    SECTION("Tokenizes basic operators") {
        Lexer lexer("+-*/(){}[];,.");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 14);  // 13 tokens + EOF
        REQUIRE(tokens[0].type == TokenType::PLUS);
        REQUIRE(tokens[1].type == TokenType::MINUS);
        REQUIRE(tokens[2].type == TokenType::STAR);
        REQUIRE(tokens[3].type == TokenType::SLASH);
        REQUIRE(tokens[4].type == TokenType::LEFT_PAREN);
        REQUIRE(tokens[5].type == TokenType::RIGHT_PAREN);
        REQUIRE(tokens[6].type == TokenType::LEFT_BRACE);
        REQUIRE(tokens[7].type == TokenType::RIGHT_BRACE);
        REQUIRE(tokens[8].type == TokenType::LEFT_BRACKET);
        REQUIRE(tokens[9].type == TokenType::RIGHT_BRACKET);
        REQUIRE(tokens[10].type == TokenType::SEMICOLON);
        REQUIRE(tokens[11].type == TokenType::COMMA);
        REQUIRE(tokens[12].type == TokenType::DOT);
        REQUIRE(tokens[13].type == TokenType::END_OF_FILE);
    }
}

TEST_CASE("Lexer tokenizes two character tokens", "[lexer]") {
    SECTION("Tokenizes comparison operators") {
        Lexer lexer("== != <= >= ->");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // 5 tokens + EOF
        REQUIRE(tokens[0].type == TokenType::EQUAL_EQUAL);
        REQUIRE(tokens[1].type == TokenType::BANG_EQUAL);
        REQUIRE(tokens[2].type == TokenType::LESS_EQUAL);
        REQUIRE(tokens[3].type == TokenType::GREATER_EQUAL);
        REQUIRE(tokens[4].type == TokenType::ARROW);
        REQUIRE(tokens[5].type == TokenType::END_OF_FILE);
    }

    SECTION("Tokenizes compound assignment operators") {
        Lexer lexer("+= -= *= /= %=");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // 5 tokens + EOF
        REQUIRE(tokens[0].type == TokenType::PLUS_EQUAL);
        REQUIRE(tokens[1].type == TokenType::MINUS_EQUAL);
        REQUIRE(tokens[2].type == TokenType::STAR_EQUAL);
        REQUIRE(tokens[3].type == TokenType::SLASH_EQUAL);
        REQUIRE(tokens[4].type == TokenType::PERCENT_EQUAL);
    }
}

TEST_CASE("Lexer tokenizes modulo operator", "[lexer]") {
    SECTION("Tokenizes percent as modulo") {
        Lexer lexer("10 % 3");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 4);  // 10 % 3 EOF
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[1].type == TokenType::PERCENT);
        REQUIRE(tokens[2].type == TokenType::NUMBER);
    }
}

TEST_CASE("Lexer tokenizes numbers", "[lexer]") {
    SECTION("Tokenizes integers") {
        Lexer lexer("123");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);  // number + EOF
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[0].lexeme == "123");
    }

    SECTION("Tokenizes floating point numbers") {
        Lexer lexer("123.456");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);  // number + EOF
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[0].lexeme == "123.456");
    }
}

TEST_CASE("Lexer tokenizes strings", "[lexer]") {
    SECTION("Tokenizes basic string") {
        Lexer lexer("\"hello world\"");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);  // string + EOF
        REQUIRE(tokens[0].type == TokenType::STRING);
        REQUIRE(tokens[0].lexeme == "\"hello world\"");
    }

    SECTION("Tokenizes empty string") {
        Lexer lexer("\"\"");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);  // string + EOF
        REQUIRE(tokens[0].type == TokenType::STRING);
        REQUIRE(tokens[0].lexeme == "\"\"");
    }
}

TEST_CASE("Lexer tokenizes identifiers and keywords", "[lexer]") {
    SECTION("Tokenizes identifiers") {
        Lexer lexer("abc xyz123 _test");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 4);  // 3 identifiers + EOF
        REQUIRE(tokens[0].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[0].lexeme == "abc");
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[1].lexeme == "xyz123");
        REQUIRE(tokens[2].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[2].lexeme == "_test");
    }

    SECTION("Tokenizes keywords") {
        Lexer lexer("var fn if else while for return break continue");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 10);  // 9 keywords + EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[1].type == TokenType::FN);
        REQUIRE(tokens[2].type == TokenType::IF);
        REQUIRE(tokens[3].type == TokenType::ELSE);
        REQUIRE(tokens[4].type == TokenType::WHILE);
        REQUIRE(tokens[5].type == TokenType::FOR);
        REQUIRE(tokens[6].type == TokenType::RETURN);
        REQUIRE(tokens[7].type == TokenType::BREAK);
        REQUIRE(tokens[8].type == TokenType::CONTINUE);
    }

    SECTION("Tokenizes boolean literals") {
        Lexer lexer("true false nil");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 4);  // 3 literals + EOF
        REQUIRE(tokens[0].type == TokenType::TRUE);
        REQUIRE(tokens[1].type == TokenType::FALSE);
        REQUIRE(tokens[2].type == TokenType::NIL);
    }
}

TEST_CASE("Lexer handles line and column tracking", "[lexer]") {
    SECTION("Tracks line numbers") {
        Lexer lexer("var x\nvar y");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens[0].line == 1);  // var
        REQUIRE(tokens[1].line == 1);  // x
        REQUIRE(tokens[2].line == 2);  // var
        REQUIRE(tokens[3].line == 2);  // y
    }
}

TEST_CASE("Lexer handles whitespace", "[lexer]") {
    SECTION("Skips whitespace between tokens") {
        Lexer lexer("  var   x  =  123  ");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 5);  // var x = 123 EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[2].type == TokenType::EQUAL);
        REQUIRE(tokens[3].type == TokenType::NUMBER);
    }
}

TEST_CASE("Lexer handles string interpolation", "[lexer]") {
    SECTION("Simple variable interpolation") {
        Lexer lexer("\"Hello, ${name}!\"");
        auto tokens = lexer.scanTokens();

        // Should tokenize to: "Hello, " + str( name ) + "!" EOF
        REQUIRE(tokens.size() == 9);
        REQUIRE(tokens[0].type == TokenType::STRING);
        REQUIRE(tokens[0].lexeme == "\"Hello, \"");
        REQUIRE(tokens[1].type == TokenType::PLUS);
        REQUIRE(tokens[2].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[2].lexeme == "str");
        REQUIRE(tokens[3].type == TokenType::LEFT_PAREN);
        REQUIRE(tokens[4].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[4].lexeme == "name");
        REQUIRE(tokens[5].type == TokenType::RIGHT_PAREN);
        REQUIRE(tokens[6].type == TokenType::PLUS);
        REQUIRE(tokens[7].type == TokenType::STRING);
        REQUIRE(tokens[7].lexeme == "\"!\"");
        REQUIRE(tokens[8].type == TokenType::END_OF_FILE);
    }

    SECTION("Multiple interpolations") {
        Lexer lexer("\"${x} and ${y}\"");
        auto tokens = lexer.scanTokens();

        // Should have: str(x) + " and " + str(y)
        REQUIRE(tokens[0].type == TokenType::IDENTIFIER);  // str
        REQUIRE(tokens[1].type == TokenType::LEFT_PAREN);
        REQUIRE(tokens[2].type == TokenType::IDENTIFIER);  // x
        REQUIRE(tokens[3].type == TokenType::RIGHT_PAREN);
        REQUIRE(tokens[4].type == TokenType::PLUS);
        REQUIRE(tokens[5].type == TokenType::STRING);  // " and "
        REQUIRE(tokens[5].lexeme == "\" and \"");
        REQUIRE(tokens[6].type == TokenType::PLUS);
        REQUIRE(tokens[7].type == TokenType::IDENTIFIER);  // str
    }

    SECTION("Expression in interpolation") {
        Lexer lexer("\"Result: ${x + y}\"");
        auto tokens = lexer.scanTokens();

        // Should tokenize expression inside interpolation
        REQUIRE(tokens[0].type == TokenType::STRING);
        REQUIRE(tokens[0].lexeme == "\"Result: \"");
        REQUIRE(tokens[1].type == TokenType::PLUS);
        REQUIRE(tokens[2].type == TokenType::IDENTIFIER);  // str
        REQUIRE(tokens[3].type == TokenType::LEFT_PAREN);
        REQUIRE(tokens[4].type == TokenType::IDENTIFIER);  // x
        REQUIRE(tokens[5].type == TokenType::PLUS);
        REQUIRE(tokens[6].type == TokenType::IDENTIFIER);  // y
        REQUIRE(tokens[7].type == TokenType::RIGHT_PAREN);
    }
}

TEST_CASE("Lexer handles single-line comments", "[lexer]") {
    SECTION("Comment at beginning of line") {
        Lexer lexer("// This is a comment\nvar x = 42;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // var x = 42 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[2].type == TokenType::EQUAL);
        REQUIRE(tokens[3].type == TokenType::NUMBER);
        REQUIRE(tokens[4].type == TokenType::SEMICOLON);
        REQUIRE(tokens[5].type == TokenType::END_OF_FILE);
    }

    SECTION("Inline comment after statement") {
        Lexer lexer("var x = 42; // inline comment");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // var x = 42 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[2].type == TokenType::EQUAL);
        REQUIRE(tokens[3].type == TokenType::NUMBER);
        REQUIRE(tokens[4].type == TokenType::SEMICOLON);
    }

    SECTION("Comment in middle of code") {
        Lexer lexer("var x = 10;\n// comment\nvar y = 20;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 11);  // var x = 10 ; var y = 20 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[4].type == TokenType::SEMICOLON);
        REQUIRE(tokens[5].type == TokenType::VAR);
    }

    SECTION("Multiple comments") {
        Lexer lexer("// First comment\n// Second comment\nvar x = 1;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // var x = 1 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
    }
}

TEST_CASE("Lexer handles multi-line comments", "[lexer]") {
    SECTION("Single line block comment") {
        Lexer lexer("var x = 10; /* comment */ var y = 20;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 11);  // var x = 10 ; var y = 20 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[1].lexeme == "x");
        REQUIRE(tokens[5].type == TokenType::VAR);
        REQUIRE(tokens[6].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[6].lexeme == "y");
    }

    SECTION("Multi-line block comment") {
        Lexer lexer("var x = 10;\n/* This is\n   a multi-line\n   comment */\nvar y = 20;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 11);  // var x = 10 ; var y = 20 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[5].type == TokenType::VAR);
    }

    SECTION("Multiple block comments") {
        Lexer lexer("/* first */ var x = 1; /* second */");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // var x = 1 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
    }

    SECTION("Block comment with special characters") {
        Lexer lexer("/* Comment with / and * and // */ var x = 1;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // var x = 1 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
    }

    SECTION("Unterminated block comment throws error") {
        Lexer lexer("/* unterminated comment");
        REQUIRE_THROWS_AS(lexer.scanTokens(), LexerError);
    }
}

TEST_CASE("Lexer handles division operator correctly", "[lexer]") {
    SECTION("Division in expression") {
        Lexer lexer("var x = 10 / 5;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 8);  // var x = 10 / 5 ; EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[3].type == TokenType::NUMBER);
        REQUIRE(tokens[3].lexeme == "10");
        REQUIRE(tokens[4].type == TokenType::SLASH);
        REQUIRE(tokens[5].type == TokenType::NUMBER);
        REQUIRE(tokens[5].lexeme == "5");
    }

    SECTION("Multiple divisions") {
        Lexer lexer("100 / 10 / 2");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 6);  // 100 / 10 / 2 EOF
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[1].type == TokenType::SLASH);
        REQUIRE(tokens[2].type == TokenType::NUMBER);
        REQUIRE(tokens[3].type == TokenType::SLASH);
        REQUIRE(tokens[4].type == TokenType::NUMBER);
    }
}

TEST_CASE("Lexer handles comments with division operator", "[lexer]") {
    SECTION("Division followed by comment") {
        Lexer lexer("var x = 10 / 5; // divide by 5");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 8);  // var x = 10 / 5 ; EOF
        REQUIRE(tokens[4].type == TokenType::SLASH);
    }

    SECTION("Comment followed by division") {
        Lexer lexer("// comment\nvar x = 10 / 5;");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 8);  // var x = 10 / 5 ; EOF
        REQUIRE(tokens[4].type == TokenType::SLASH);
    }
}

TEST_CASE("Lexer tracks column numbers correctly", "[lexer]") {
    SECTION("Tokens on first line have correct columns") {
        Lexer lexer("var x = 42");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens[0].column == 1);   // var
        REQUIRE(tokens[1].column == 5);   // x
        REQUIRE(tokens[2].column == 7);   // =
        REQUIRE(tokens[3].column == 9);   // 42
    }

    SECTION("Tokens on subsequent lines start at column 1") {
        Lexer lexer("var x\nvar y");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens[0].column == 1);  // var (line 1)
        REQUIRE(tokens[1].column == 5);  // x   (line 1)
        REQUIRE(tokens[2].column == 1);  // var (line 2)
        REQUIRE(tokens[3].column == 5);  // y   (line 2)
    }

    SECTION("Compound tokens report start column") {
        Lexer lexer("x == y");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens[0].column == 1);  // x
        REQUIRE(tokens[1].column == 3);  // ==
        REQUIRE(tokens[2].column == 6);  // y
    }

    SECTION("Compound assignment operators report correct column") {
        Lexer lexer("x += 1");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens[0].column == 1);  // x
        REQUIRE(tokens[1].column == 3);  // +=
        REQUIRE(tokens[2].column == 6);  // 1
    }

    SECTION("Arrow operator reports correct column") {
        Lexer lexer("fn -> x");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens[0].column == 1);  // fn
        REQUIRE(tokens[1].column == 4);  // ->
        REQUIRE(tokens[2].column == 7);  // x
    }
}

TEST_CASE("Lexer tokenizes remaining single-character tokens", "[lexer]") {
    SECTION("Tokenizes colon") {
        Lexer lexer("x : y");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 4);
        REQUIRE(tokens[1].type == TokenType::COLON);
    }

    SECTION("Tokenizes question mark") {
        Lexer lexer("x ? y");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 4);
        REQUIRE(tokens[1].type == TokenType::QUESTION);
    }

    SECTION("Tokenizes spread operator") {
        Lexer lexer("...");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::DOT_DOT_DOT);
        REQUIRE(tokens[0].lexeme == "...");
    }
}

TEST_CASE("Lexer tokenizes fat-arrow operator", "[lexer]") {
    SECTION("Tokenizes => as ARROW") {
        Lexer lexer("=> x");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].type == TokenType::ARROW);
        REQUIRE(tokens[0].lexeme == "=>");
    }
}

TEST_CASE("Lexer tokenizes underscore wildcard", "[lexer]") {
    SECTION("Standalone underscore is UNDERSCORE token") {
        Lexer lexer("_");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::UNDERSCORE);
        REQUIRE(tokens[0].lexeme == "_");
    }

    SECTION("Identifier starting with underscore is IDENTIFIER") {
        Lexer lexer("_name");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[0].lexeme == "_name");
    }
}

TEST_CASE("Lexer tokenizes all keywords", "[lexer]") {
    SECTION("Tokenizes OOP keywords") {
        Lexer lexer("class extends this super");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].type == TokenType::CLASS);
        REQUIRE(tokens[1].type == TokenType::EXTENDS);
        REQUIRE(tokens[2].type == TokenType::THIS);
        REQUIRE(tokens[3].type == TokenType::SUPER);
    }

    SECTION("Tokenizes module keywords") {
        Lexer lexer("import export from as");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].type == TokenType::IMPORT);
        REQUIRE(tokens[1].type == TokenType::EXPORT);
        REQUIRE(tokens[2].type == TokenType::FROM);
        REQUIRE(tokens[3].type == TokenType::AS);
    }

    SECTION("Tokenizes exception keywords") {
        Lexer lexer("try catch finally throw");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].type == TokenType::TRY);
        REQUIRE(tokens[1].type == TokenType::CATCH);
        REQUIRE(tokens[2].type == TokenType::FINALLY);
        REQUIRE(tokens[3].type == TokenType::THROW);
    }

    SECTION("Tokenizes async/await keywords") {
        Lexer lexer("async await");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].type == TokenType::ASYNC);
        REQUIRE(tokens[1].type == TokenType::AWAIT);
    }

    SECTION("Tokenizes match and default keywords") {
        Lexer lexer("match default");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].type == TokenType::MATCH);
        REQUIRE(tokens[1].type == TokenType::DEFAULT);
    }

    SECTION("Tokenizes macro keyword") {
        Lexer lexer("macro");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].type == TokenType::MACRO);
    }

    SECTION("Tokenizes logical keywords") {
        Lexer lexer("and or");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].type == TokenType::AND);
        REQUIRE(tokens[1].type == TokenType::OR);
    }
}

TEST_CASE("Lexer handles unexpected characters gracefully", "[lexer]") {
    SECTION("Throws LexerError for unexpected character") {
        Lexer lexer("var x = @bad;");
        REQUIRE_THROWS_AS(lexer.scanTokens(), LexerError);
    }

    SECTION("LexerError has correct line and column") {
        Lexer lexer("var x = @;");
        try {
            lexer.scanTokens();
            FAIL("Expected LexerError");
        } catch (const LexerError& e) {
            REQUIRE(e.line == 1);
            REQUIRE(e.column == 9);
        }
    }

    SECTION("Throws LexerError for unterminated string") {
        Lexer lexer("\"unterminated");
        REQUIRE_THROWS_AS(lexer.scanTokens(), LexerError);
    }
}

TEST_CASE("Lexer tokenizes representative program", "[lexer]") {
    SECTION("Full function declaration") {
        Lexer lexer("fn add(x, y) { return x + y; }");
        auto tokens = lexer.scanTokens();

        REQUIRE(tokens[0].type == TokenType::FN);
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[1].lexeme == "add");
        REQUIRE(tokens[2].type == TokenType::LEFT_PAREN);
        REQUIRE(tokens[3].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[3].lexeme == "x");
        REQUIRE(tokens[4].type == TokenType::COMMA);
        REQUIRE(tokens[5].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[5].lexeme == "y");
        REQUIRE(tokens[6].type == TokenType::RIGHT_PAREN);
        REQUIRE(tokens[7].type == TokenType::LEFT_BRACE);
        REQUIRE(tokens[8].type == TokenType::RETURN);
        REQUIRE(tokens[9].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[10].type == TokenType::PLUS);
        REQUIRE(tokens[11].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[12].type == TokenType::SEMICOLON);
        REQUIRE(tokens[13].type == TokenType::RIGHT_BRACE);
        REQUIRE(tokens[14].type == TokenType::END_OF_FILE);
    }

    SECTION("Multi-line program with comments") {
        Lexer lexer(
            "// greeting\n"
            "var name = \"world\";\n"
            "/* say hello */\n"
            "var msg = \"Hello, \" + name;\n");
        auto tokens = lexer.scanTokens();

        // Check key tokens
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[0].line == 2);
        REQUIRE(tokens[0].column == 1);
        REQUIRE(tokens[1].lexeme == "name");
        REQUIRE(tokens[4].type == TokenType::SEMICOLON);
        REQUIRE(tokens[5].type == TokenType::VAR);
        REQUIRE(tokens[5].line == 4);
    }
}

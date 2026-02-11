#include "catch.hpp"
#include "parse/lexer.hpp"
#include "common/token.hpp"

using namespace izi;

TEST_CASE("Lexer tokenizes single character tokens", "[lexer]") {
    SECTION("Tokenizes basic operators") {
        Lexer lexer("+-*/(){}[];,.");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens.size() == 14); // 13 tokens + EOF
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
        
        REQUIRE(tokens.size() == 6); // 5 tokens + EOF
        REQUIRE(tokens[0].type == TokenType::EQUAL_EQUAL);
        REQUIRE(tokens[1].type == TokenType::BANG_EQUAL);
        REQUIRE(tokens[2].type == TokenType::LESS_EQUAL);
        REQUIRE(tokens[3].type == TokenType::GREATER_EQUAL);
        REQUIRE(tokens[4].type == TokenType::ARROW);
        REQUIRE(tokens[5].type == TokenType::END_OF_FILE);
    }
}

TEST_CASE("Lexer tokenizes numbers", "[lexer]") {
    SECTION("Tokenizes integers") {
        Lexer lexer("123");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens.size() == 2); // number + EOF
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[0].lexeme == "123");
    }
    
    SECTION("Tokenizes floating point numbers") {
        Lexer lexer("123.456");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens.size() == 2); // number + EOF
        REQUIRE(tokens[0].type == TokenType::NUMBER);
        REQUIRE(tokens[0].lexeme == "123.456");
    }
}

TEST_CASE("Lexer tokenizes strings", "[lexer]") {
    SECTION("Tokenizes basic string") {
        Lexer lexer("\"hello world\"");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens.size() == 2); // string + EOF
        REQUIRE(tokens[0].type == TokenType::STRING);
        REQUIRE(tokens[0].lexeme == "\"hello world\"");
    }
    
    SECTION("Tokenizes empty string") {
        Lexer lexer("\"\"");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens.size() == 2); // string + EOF
        REQUIRE(tokens[0].type == TokenType::STRING);
        REQUIRE(tokens[0].lexeme == "\"\"");
    }
}

TEST_CASE("Lexer tokenizes identifiers and keywords", "[lexer]") {
    SECTION("Tokenizes identifiers") {
        Lexer lexer("abc xyz123 _test");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens.size() == 4); // 3 identifiers + EOF
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
        
        REQUIRE(tokens.size() == 10); // 9 keywords + EOF
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
        
        REQUIRE(tokens.size() == 4); // 3 literals + EOF
        REQUIRE(tokens[0].type == TokenType::TRUE);
        REQUIRE(tokens[1].type == TokenType::FALSE);
        REQUIRE(tokens[2].type == TokenType::NIL);
    }
}

TEST_CASE("Lexer handles line and column tracking", "[lexer]") {
    SECTION("Tracks line numbers") {
        Lexer lexer("var x\nvar y");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens[0].line == 1); // var
        REQUIRE(tokens[1].line == 1); // x
        REQUIRE(tokens[2].line == 2); // var
        REQUIRE(tokens[3].line == 2); // y
    }
}

TEST_CASE("Lexer handles whitespace", "[lexer]") {
    SECTION("Skips whitespace between tokens") {
        Lexer lexer("  var   x  =  123  ");
        auto tokens = lexer.scanTokens();
        
        REQUIRE(tokens.size() == 5); // var x = 123 EOF
        REQUIRE(tokens[0].type == TokenType::VAR);
        REQUIRE(tokens[1].type == TokenType::IDENTIFIER);
        REQUIRE(tokens[2].type == TokenType::EQUAL);
        REQUIRE(tokens[3].type == TokenType::NUMBER);
    }
}

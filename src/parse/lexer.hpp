#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "common/token.hpp"

namespace izi {

class Lexer {
   public:
    explicit Lexer(std::string source) : source(std::move(source)) {}

    std::vector<Token> scanTokens();

   private:
    void scanToken();
    void addToken(TokenType type);
    void addToken(TokenType type, std::string_view literal);

    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);

    void string();
    void number();
    void identifier();

    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;

    TokenType identifierType() const;
    TokenType checkKeyword(int start, int length, std::string_view rest, TokenType type) const;
    static TokenType keywordType(std::string_view text);

    std::string source;
    std::vector<Token> tokens;
    int start = 0;
    int current = 0;
    int line = 1;
    int column = 1;
};

}  // namespace izi
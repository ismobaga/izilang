#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "common/diagnostics.hpp"
#include "common/token.hpp"

namespace izi {

class LexerError : public std::runtime_error {
   public:
    int line;
    int column;

    LexerError(int line, int column, const std::string& message)
        : std::runtime_error(message), line(line), column(column) {}
};

class Lexer {
   public:
    explicit Lexer(std::string source, DiagnosticEngine* diags = nullptr)
        : source(std::move(source)), diags_(diags) {}

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
    void skipLineComment();
    void skipBlockComment();

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
    int startLine = 1;   // Line where current token starts
    int startColumn = 1; // Column where current token starts
    DiagnosticEngine* diags_ = nullptr;  // optional, not owned
};

}  // namespace izi
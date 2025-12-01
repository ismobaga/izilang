#pragma once

#include <initializer_list>
#include <memory>
#include <vector>

#include "ast/stmt.hpp"
#include "common/token.hpp"

namespace izi {
class Parser {
   public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

    std::vector<StmtPtr> parse();

   private:
    // Expression parsing (precedence climbing)
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call() ;

    ExprPtr primary();

    // Statement parsing
    StmtPtr statement();
    StmtPtr declaration();
    StmtPtr varDeclaration();
    StmtPtr expressionStatement();
    StmtPtr printStatement();
    StmtPtr blockStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr forStatement();
    StmtPtr returnStatement();
    StmtPtr functionDeclaration();

    // Helper methods
    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
    void synchronize();

    std::runtime_error error(const Token& token, const std::string& message);

    std::vector<Token> tokens;
    size_t current = 0;
};
}  // namespace izi

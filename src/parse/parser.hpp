#pragma once

#include <initializer_list>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "ast/stmt.hpp"
#include "common/token.hpp"

namespace izi {

class ParserError : public std::runtime_error {
   public:
    Token token;

    ParserError(const Token& token, const std::string& message) : std::runtime_error(message), token(token) {}
};

// Stores a macro definition: parameter names + body tokens
struct MacroDefinition {
    std::vector<std::string> params;
    std::vector<Token> bodyTokens;
};

class Parser {
   public:
    explicit Parser(std::vector<Token> tokens, std::string_view source = "")
        : tokens(std::move(tokens)), source_(source) {}

    std::vector<StmtPtr> parse();

   private:
    // Expression parsing (precedence climbing)
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr conditional();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();

    ExprPtr primary();

    // Pattern parsing for match expressions
    PatternPtr parsePattern();

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
    StmtPtr breakStatement();
    StmtPtr continueStatement();
    StmtPtr functionDeclaration();
    StmtPtr importStatement();
    StmtPtr exportStatement();
    StmtPtr tryStatement();
    StmtPtr throwStatement();
    StmtPtr classDeclaration();

    // Macro support
    StmtPtr macroDeclaration();
    ExprPtr expandMacroExpr(const std::string& macroName);
    StmtPtr expandMacroStmt();
    std::vector<std::vector<Token>> collectMacroArgs(size_t expectedCount, const Token& callToken);
    std::vector<Token> substituteBody(const MacroDefinition& macro,
                                      const std::vector<std::vector<Token>>& args,
                                      int line, int col);

    // Type annotation parsing (v0.3)
    TypePtr parseTypeAnnotation();

    // Helper methods
    bool match(std::initializer_list<TokenType> types);
    bool check(TokenType type) const;
    Token advance();
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token consume(TokenType type, const std::string& message);
    void consumeSemicolonIfNeeded();
    void synchronize();

    ParserError error(const Token& token, const std::string& message);

    std::vector<Token> tokens;
    size_t current = 0;
    std::string_view source_;

    // Macro definitions accumulated during parsing
    std::unordered_map<std::string, MacroDefinition> macros_;
};
}  // namespace izi
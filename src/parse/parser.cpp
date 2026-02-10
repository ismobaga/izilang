#include "parser.hpp"

#include <stdexcept>
#include <utility>

#include "ast/expr.hpp"
#include "ast/stmt.hpp"

namespace izi {

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

// Statement parsing
StmtPtr Parser::declaration() {
    try {
        if (match({TokenType::VAR})) return varDeclaration();
        if (match({TokenType::FN})) return functionDeclaration();
        if (match({TokenType::IMPORT})) return importStatement();
        return statement();
    } catch (const std::runtime_error& e) {
        synchronize();
        return nullptr;
    }
}

StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    ExprPtr initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VarStmt>(std::string(name.lexeme), std::move(initializer));
}

StmtPtr Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    std::vector<std::string> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token param = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back(std::string(param.lexeme));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TokenType::LEFT_BRACE, "Expect '{' before function body.");
    auto body = blockStatement();

    // Extract statements from BlockStmt
    auto* blockPtr = dynamic_cast<BlockStmt*>(body.get());
    std::vector<StmtPtr> bodyStmts;
    if (blockPtr) {
        bodyStmts = std::move(blockPtr->statements);
    }

    return std::make_unique<FunctionStmt>(
        std::string(name.lexeme),
        std::move(params),
        std::move(bodyStmts));
}

StmtPtr Parser::importStatement() {
    Token moduleToken = consume(TokenType::STRING, "Expect module name as string.");
    consume(TokenType::SEMICOLON, "Expect ';' after import statement.");
    // Remove quotes from string literal
    std::string moduleName = std::string(moduleToken.lexeme);
    if (moduleName.size() >= 2 && moduleName.front() == '"' && moduleName.back() == '"') {
        moduleName = moduleName.substr(1, moduleName.size() - 2);
    }
    return std::make_unique<ImportStmt>(std::move(moduleName));
}

StmtPtr Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::LEFT_BRACE})) return blockStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after value.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

StmtPtr Parser::blockStatement() {
    std::vector<StmtPtr> statements;

    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

StmtPtr Parser::ifStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after if condition.");

    StmtPtr thenBranch = statement();
    StmtPtr elseBranch = nullptr;
    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch));
}

StmtPtr Parser::whileStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    ExprPtr condition = expression();
    consume(TokenType::RIGHT_PAREN, "Expect ')' after condition.");
    StmtPtr body = statement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

StmtPtr Parser::forStatement() {
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'for'.");

    // Initializer
    StmtPtr initializer;
    if (match({TokenType::SEMICOLON})) {
        initializer = nullptr;
    } else if (match({TokenType::VAR})) {
        initializer = varDeclaration();
    } else {
        initializer = expressionStatement();
    }

    // Condition
    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after loop condition.");

    // Increment
    ExprPtr increment = nullptr;
    if (!check(TokenType::RIGHT_PAREN)) {
        increment = expression();
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after for clauses.");

    StmtPtr body = statement();

    // Desugar for loop into while loop
    if (increment != nullptr) {
        std::vector<StmtPtr> bodyWithIncrement;
        bodyWithIncrement.push_back(std::move(body));
        bodyWithIncrement.push_back(std::make_unique<ExprStmt>(std::move(increment)));
        body = std::make_unique<BlockStmt>(std::move(bodyWithIncrement));
    }

    if (condition == nullptr) {
        condition = std::make_unique<LiteralExpr>(true);
    }
    body = std::make_unique<WhileStmt>(std::move(condition), std::move(body));

    if (initializer != nullptr) {
        std::vector<StmtPtr> withInitializer;
        withInitializer.push_back(std::move(initializer));
        withInitializer.push_back(std::move(body));
        body = std::make_unique<BlockStmt>(std::move(withInitializer));
    }

    return body;
}

StmtPtr Parser::returnStatement() {
    ExprPtr value = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(std::move(value));
}

// Expression parsing (precedence climbing)
ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = equality();

    if (match({TokenType::EQUAL})) {
        const Token& equals = previous();
        ExprPtr value = assignment();

        // Variable for now
        if (auto var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(var->name, std::move(value));
        }
        if (auto indexExpr = dynamic_cast<IndexExpr*>(expr.get())) {
            return std::make_unique<SetIndexExpr>(std::move(indexExpr->collection), std::move(indexExpr->index), std::move(value));
        }

        throw error(equals, "Invalid assignment target");
    }

    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL,
                  TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        ExprPtr right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        ExprPtr right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }

    return call();
}
ExprPtr Parser::call() {
    ExprPtr expr = primary();

    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            std::vector<ExprPtr> args;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    args.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        } else  if(match({TokenType::LEFT_BRACKET})){
            // Index : expr[ expression ]
            ExprPtr index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index expression.");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));

        }
        else
        {
            break;
        }
    }

    return expr;
}
ExprPtr Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_unique<LiteralExpr>(false);
    if (match({TokenType::TRUE})) return std::make_unique<LiteralExpr>(true);
    if (match({TokenType::NIL})) return std::make_unique<LiteralExpr>(Nil{});

    if (match({TokenType::NUMBER})) {
        double value = std::stod(std::string(previous().lexeme));
        return std::make_unique<LiteralExpr>(value);
    }

    if (match({TokenType::STRING})) {
        std::string value(previous().lexeme);
        // Remove surrounding quotes
        if (value.length() >= 2) {
            value = value.substr(1, value.length() - 2);
        }
        return std::make_unique<LiteralExpr>(value);
    }

    if (match({TokenType::IDENTIFIER})) {
        Token name = previous();
        return std::make_unique<VariableExpr>(std::string(name.lexeme), nullptr);

    }
    if (match({TokenType::LEFT_BRACKET})) {
        // Array literal
        std::vector<ExprPtr> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                elements.push_back(expression());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    if(match({TokenType::LEFT_BRACE})){
        // Map literal
        std::vector<std::pair<std::string, ExprPtr>> entries;
        if (!check(TokenType::RIGHT_BRACE)) {
            do {
                Token keyToken = consume(TokenType::STRING, "Expect string as map key.");
                std::string key = std::string(keyToken.lexeme);
                // Remove surrounding quotes
                if (key.length() >= 2) {
                    key = key.substr(1, key.length() - 2);
                }
                consume(TokenType::COLON, "Expect ':' after map key.");
                ExprPtr value = expression();
                entries.emplace_back(std::move(key), std::move(value));
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after map entries.");
        return std::make_unique<MapExpr>(std::move(entries));
    }

    if (match({TokenType::LEFT_PAREN})) {
        ExprPtr expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    throw error(peek(), "Expect expression.");
}

// Helper methods
bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;

        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default:
                break;
        }

        advance();
    }
}

ParserError Parser::error(const Token& token, const std::string& message) {
    return ParserError(token, message);
}

}  // namespace izi

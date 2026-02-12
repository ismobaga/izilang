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
        if (match({TokenType::CLASS})) return classDeclaration();
        if (match({TokenType::IMPORT})) return importStatement();
        if (match({TokenType::EXPORT})) return exportStatement();
        return statement();
    } catch (const std::runtime_error& e) {
        synchronize();
        return nullptr;
    }
}

StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    // Parse optional type annotation
    TypePtr typeAnnotation = nullptr;
    if (match({TokenType::COLON})) {
        typeAnnotation = parseTypeAnnotation();
    }

    ExprPtr initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }

    consumeSemicolonIfNeeded();
    return std::make_unique<VarStmt>(std::string(name.lexeme), std::move(initializer), std::move(typeAnnotation));
}

StmtPtr Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");

    std::vector<std::string> params;
    std::vector<TypePtr> paramTypes;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token param = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back(std::string(param.lexeme));
            
            // Parse optional parameter type annotation
            TypePtr paramType = nullptr;
            if (match({TokenType::COLON})) {
                paramType = parseTypeAnnotation();
            }
            paramTypes.push_back(std::move(paramType));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");

    // Parse optional return type annotation
    TypePtr returnType = nullptr;
    if (match({TokenType::COLON})) {
        returnType = parseTypeAnnotation();
    }

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
        std::move(bodyStmts),
        std::move(paramTypes),
        std::move(returnType));
}

StmtPtr Parser::importStatement() {
    // Three possible syntaxes:
    // 1. import "module.iz";  (simple import - backward compatible)
    // 2. import { name1, name2 } from "module.iz";  (named imports)
    // 3. import * as alias from "module.iz";  (wildcard import)
    
    // Check for wildcard import: import *
    if (match({TokenType::STAR})) {
        consume(TokenType::AS, "Expect 'as' after '*' in import statement.");
        Token alias = consume(TokenType::IDENTIFIER, "Expect alias name after 'as'.");
        consume(TokenType::FROM, "Expect 'from' after alias in import statement.");
        Token moduleToken = consume(TokenType::STRING, "Expect module name as string.");
        consumeSemicolonIfNeeded();
        
        // Remove quotes from string literal
        std::string moduleName = std::string(moduleToken.lexeme);
        if (moduleName.size() >= 2 && moduleName.front() == '"' && moduleName.back() == '"') {
            moduleName = moduleName.substr(1, moduleName.size() - 2);
        }
        
        return std::make_unique<ImportStmt>(std::move(moduleName), std::string(alias.lexeme), true);
    }
    
    // Check for named imports: import {
    if (match({TokenType::LEFT_BRACE})) {
        std::vector<std::string> namedImports;
        do {
            Token name = consume(TokenType::IDENTIFIER, "Expect identifier in import list.");
            namedImports.push_back(std::string(name.lexeme));
        } while (match({TokenType::COMMA}));
        
        consume(TokenType::RIGHT_BRACE, "Expect '}' after import list.");
        consume(TokenType::FROM, "Expect 'from' after import list.");
        Token moduleToken = consume(TokenType::STRING, "Expect module name as string.");
        consumeSemicolonIfNeeded();
        
        // Remove quotes from string literal
        std::string moduleName = std::string(moduleToken.lexeme);
        if (moduleName.size() >= 2 && moduleName.front() == '"' && moduleName.back() == '"') {
            moduleName = moduleName.substr(1, moduleName.size() - 2);
        }
        
        return std::make_unique<ImportStmt>(std::move(moduleName), std::move(namedImports));
    }
    
    // Simple import: import "module.iz";
    Token moduleToken = consume(TokenType::STRING, "Expect module name as string.");
    consumeSemicolonIfNeeded();
    
    // Remove quotes from string literal
    std::string moduleName = std::string(moduleToken.lexeme);
    if (moduleName.size() >= 2 && moduleName.front() == '"' && moduleName.back() == '"') {
        moduleName = moduleName.substr(1, moduleName.size() - 2);
    }
    
    return std::make_unique<ImportStmt>(std::move(moduleName));
}

StmtPtr Parser::exportStatement() {
    // Export must be followed by either 'fn' or 'var'
    // export fn name() { ... }
    // export var name = value;
    
    if (match({TokenType::FN})) {
        auto funcDecl = functionDeclaration();
        return std::make_unique<ExportStmt>(std::move(funcDecl));
    }
    
    if (match({TokenType::VAR})) {
        auto varDecl = varDeclaration();
        return std::make_unique<ExportStmt>(std::move(varDecl));
    }
    
    throw error(peek(), "Expect 'fn' or 'var' after 'export'.");
}

StmtPtr Parser::statement() {
    if (match({TokenType::PRINT})) return printStatement();
    if (match({TokenType::LEFT_BRACE})) return blockStatement();
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::FOR})) return forStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::BREAK})) return breakStatement();
    if (match({TokenType::CONTINUE})) return continueStatement();
    if (match({TokenType::TRY})) return tryStatement();
    if (match({TokenType::THROW})) return throwStatement();
    return expressionStatement();
}

StmtPtr Parser::printStatement() {
    ExprPtr expr = expression();
    consumeSemicolonIfNeeded();
    return std::make_unique<ExprStmt>(std::move(expr));
}

StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consumeSemicolonIfNeeded();
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
    if (!isAtEnd() && peek().line == previous().line) {
        // Check if there's actually an expression (not just a semicolon or newline)
        if (!check(TokenType::SEMICOLON)) {
            value = expression();
        }
    }
    consumeSemicolonIfNeeded();
    return std::make_unique<ReturnStmt>(std::move(value));
}

StmtPtr Parser::breakStatement() {
    consumeSemicolonIfNeeded();
    return std::make_unique<BreakStmt>();
}

StmtPtr Parser::continueStatement() {
    consumeSemicolonIfNeeded();
    return std::make_unique<ContinueStmt>();
}

StmtPtr Parser::tryStatement() {
    // Parse try block
    consume(TokenType::LEFT_BRACE, "Expect '{' after 'try'.");
    StmtPtr tryBlock = blockStatement();
    
    // Parse optional catch block
    StmtPtr catchBlock = nullptr;
    std::string catchVariable;
    if (match({TokenType::CATCH})) {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'catch'.");
        Token varToken = consume(TokenType::IDENTIFIER, "Expect variable name in catch.");
        catchVariable = std::string(varToken.lexeme);
        consume(TokenType::RIGHT_PAREN, "Expect ')' after catch variable.");
        consume(TokenType::LEFT_BRACE, "Expect '{' after catch clause.");
        catchBlock = blockStatement();
    }
    
    // Parse optional finally block
    StmtPtr finallyBlock = nullptr;
    if (match({TokenType::FINALLY})) {
        consume(TokenType::LEFT_BRACE, "Expect '{' after 'finally'.");
        finallyBlock = blockStatement();
    }
    
    // Must have at least catch or finally
    if (catchBlock == nullptr && finallyBlock == nullptr) {
        throw error(previous(), "Expected 'catch' or 'finally' after 'try' block.");
    }
    
    return std::make_unique<TryStmt>(
        std::move(tryBlock),
        std::move(catchVariable),
        std::move(catchBlock),
        std::move(finallyBlock));
}

StmtPtr Parser::throwStatement() {
    Token throwToken = previous();  // Get the 'throw' token
    ExprPtr value = expression();
    consumeSemicolonIfNeeded();
    return std::make_unique<ThrowStmt>(std::move(throwToken), std::move(value));
}

StmtPtr Parser::classDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect class name.");
    
    // Check for extends clause
    std::string superclass;
    if (match({TokenType::EXTENDS})) {
        Token superName = consume(TokenType::IDENTIFIER, "Expect superclass name.");
        superclass = std::string(superName.lexeme);
    }
    
    consume(TokenType::LEFT_BRACE, "Expect '{' after class name.");
    
    std::vector<std::unique_ptr<VarStmt>> fields;
    std::vector<std::unique_ptr<FunctionStmt>> methods;
    
    // Parse class body (fields and methods)
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        if (match({TokenType::VAR})) {
            // Parse field declaration
            Token fieldName = consume(TokenType::IDENTIFIER, "Expect field name.");
            
            // Parse optional type annotation
            TypePtr typeAnnotation = nullptr;
            if (match({TokenType::COLON})) {
                typeAnnotation = parseTypeAnnotation();
            }
            
            // Optional initializer
            ExprPtr initializer = nullptr;
            if (match({TokenType::EQUAL})) {
                initializer = expression();
            }
            
            consumeSemicolonIfNeeded();
            fields.push_back(std::make_unique<VarStmt>(
                std::string(fieldName.lexeme), 
                std::move(initializer), 
                std::move(typeAnnotation)
            ));
        } else if (match({TokenType::FN})) {
            // Parse method declaration
            Token methodName = consume(TokenType::IDENTIFIER, "Expect method name.");
            consume(TokenType::LEFT_PAREN, "Expect '(' after method name.");
            
            std::vector<std::string> params;
            std::vector<TypePtr> paramTypes;
            if (!check(TokenType::RIGHT_PAREN)) {
                do {
                    Token param = consume(TokenType::IDENTIFIER, "Expect parameter name.");
                    params.push_back(std::string(param.lexeme));
                    
                    // Parse optional parameter type annotation
                    TypePtr paramType = nullptr;
                    if (match({TokenType::COLON})) {
                        paramType = parseTypeAnnotation();
                    }
                    paramTypes.push_back(std::move(paramType));
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
            
            // Parse optional return type annotation
            TypePtr returnType = nullptr;
            if (match({TokenType::COLON})) {
                returnType = parseTypeAnnotation();
            }
            
            consume(TokenType::LEFT_BRACE, "Expect '{' before method body.");
            auto body = blockStatement();
            
            // Extract statements from BlockStmt
            auto* blockPtr = dynamic_cast<BlockStmt*>(body.get());
            std::vector<StmtPtr> bodyStmts;
            if (blockPtr) {
                bodyStmts = std::move(blockPtr->statements);
            }
            
            methods.push_back(std::make_unique<FunctionStmt>(
                std::string(methodName.lexeme),
                std::move(params),
                std::move(bodyStmts),
                std::move(paramTypes),
                std::move(returnType)
            ));
        } else {
            throw error(peek(), "Expect 'var' or 'fn' in class body.");
        }
    }
    
    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    return std::make_unique<ClassStmt>(
        std::string(name.lexeme),
        std::move(superclass),
        std::move(fields),
        std::move(methods)
    );
}

// Type annotation parsing (v0.3)
TypePtr Parser::parseTypeAnnotation() {
    // Check for basic types
    if (match({TokenType::IDENTIFIER})) {
        std::string typeName = std::string(previous().lexeme);
        
        // Check for generic types (Array<T>, Map<K,V>)
        if (match({TokenType::LESS})) {
            if (typeName == "Array") {
                TypePtr elementType = parseTypeAnnotation();
                consume(TokenType::GREATER, "Expect '>' after array element type.");
                return TypeAnnotation::array(std::move(elementType));
            } else if (typeName == "Map") {
                TypePtr keyType = parseTypeAnnotation();
                consume(TokenType::COMMA, "Expect ',' between map key and value types.");
                TypePtr valueType = parseTypeAnnotation();
                consume(TokenType::GREATER, "Expect '>' after map value type.");
                return TypeAnnotation::map(std::move(keyType), std::move(valueType));
            } else {
                throw error(previous(), "Unknown generic type '" + typeName + "'.");
            }
        }
        
        // Simple types
        if (typeName == "Number") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Number);
        } else if (typeName == "String") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::String);
        } else if (typeName == "Bool") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Bool);
        } else if (typeName == "Nil") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Nil);
        } else if (typeName == "Any") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Any);
        } else if (typeName == "Void") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Void);
        } else {
            throw error(previous(), "Unknown type '" + typeName + "'.");
        }
    }
    
    // Check for function types: fn(T1, T2, ...) -> R
    if (match({TokenType::FN})) {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'fn' in function type.");
        
        std::vector<TypePtr> paramTypes;
        if (!check(TokenType::RIGHT_PAREN)) {
            do {
                paramTypes.push_back(parseTypeAnnotation());
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_PAREN, "Expect ')' after function parameter types.");
        
        // Parse return type
        TypePtr returnType = TypeAnnotation::simple(TypeAnnotation::Kind::Void);
        if (match({TokenType::ARROW})) {
            returnType = parseTypeAnnotation();
        }
        
        return TypeAnnotation::function(std::move(paramTypes), std::move(returnType));
    }
    
    throw error(peek(), "Expect type annotation.");
}

// Expression parsing (precedence climbing)
ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = logicalOr();

    if (match({TokenType::EQUAL})) {
        const Token& equals = previous();
        ExprPtr value = assignment();

        // Variable assignment
        if (auto var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(var->name, std::move(value));
        }
        // Index assignment: arr[i] = value
        if (auto indexExpr = dynamic_cast<IndexExpr*>(expr.get())) {
            return std::make_unique<SetIndexExpr>(std::move(indexExpr->collection), std::move(indexExpr->index), std::move(value));
        }
        // Property assignment: obj.prop = value (v0.3)
        if (auto propExpr = dynamic_cast<PropertyExpr*>(expr.get())) {
            return std::make_unique<SetPropertyExpr>(std::move(propExpr->object), propExpr->property, std::move(value));
        }

        throw error(equals, "Invalid assignment target");
    }

    return expr;
}

ExprPtr Parser::logicalOr() {
    ExprPtr expr = logicalAnd();

    while (match({TokenType::OR})) {
        Token op = previous();
        ExprPtr right = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

ExprPtr Parser::logicalAnd() {
    ExprPtr expr = equality();

    while (match({TokenType::AND})) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
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
        } else if (match({TokenType::LEFT_BRACKET})) {
            // Index : expr[ expression ]
            ExprPtr index = expression();
            consume(TokenType::RIGHT_BRACKET, "Expect ']' after index expression.");
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else if (match({TokenType::DOT})) {
            // Property access: expr.property (v0.3)
            Token property = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr = std::make_unique<PropertyExpr>(std::move(expr), std::string(property.lexeme));
        } else {
            break;
        }
    }

    return expr;
}
ExprPtr Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_unique<LiteralExpr>(false);
    if (match({TokenType::TRUE})) return std::make_unique<LiteralExpr>(true);
    if (match({TokenType::NIL})) return std::make_unique<LiteralExpr>(Nil{});
    if (match({TokenType::THIS})) return std::make_unique<ThisExpr>();  // v0.3
    
    // Super expression: super.method
    if (match({TokenType::SUPER})) {
        consume(TokenType::DOT, "Expect '.' after 'super'.");
        Token method = consume(TokenType::IDENTIFIER, "Expect method name after 'super.'.");
        return std::make_unique<SuperExpr>(std::string(method.lexeme));
    }

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

    // Function expression: fn(params) { body }
    if (match({TokenType::FN})) {
        consume(TokenType::LEFT_PAREN, "Expect '(' after 'fn' in function expression.");
        
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
        
        return std::make_unique<FunctionExpr>(std::move(params), std::move(bodyStmts));
    }

    // Match expression: match value { pattern => result, ... }
    if (match({TokenType::MATCH})) {
        ExprPtr value = expression();
        consume(TokenType::LEFT_BRACE, "Expect '{' after match value.");
        
        std::vector<MatchCase> cases;
        
        // Parse match cases
        while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
            // Parse pattern
            PatternPtr pattern = parsePattern();
            
            // Parse optional guard: if condition
            ExprPtr guard = nullptr;
            if (match({TokenType::IF})) {
                guard = expression();
            }
            
            // Expect => arrow
            consume(TokenType::ARROW, "Expect '=>' after match pattern.");
            
            // Parse result expression
            ExprPtr result = expression();
            
            cases.emplace_back(std::move(pattern), std::move(guard), std::move(result));
            
            // Consume comma if present (allows trailing comma and optional commas)
            if (!check(TokenType::RIGHT_BRACE)) {
                match({TokenType::COMMA});  // Comma is optional
            }
        }
        
        consume(TokenType::RIGHT_BRACE, "Expect '}' after match cases.");
        return std::make_unique<MatchExpr>(std::move(value), std::move(cases));
    }

    if (match({TokenType::IDENTIFIER})) {
        Token name = previous();
        return std::make_unique<VariableExpr>(std::string(name.lexeme), nullptr);

    }
    if (match({TokenType::LEFT_BRACKET})) {
        // Array literal with potential spread elements
        std::vector<ExprPtr> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                // Check for spread operator
                if (match({TokenType::DOT_DOT_DOT})) {
                    ExprPtr spreadArg = expression();
                    elements.push_back(std::make_unique<SpreadExpr>(std::move(spreadArg)));
                } else {
                    elements.push_back(expression());
                }
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array elements.");
        return std::make_unique<ArrayExpr>(std::move(elements));
    }

    if(match({TokenType::LEFT_BRACE})){
        // Map literal with potential spread elements
        std::vector<std::pair<std::string, ExprPtr>> entries;
        if (!check(TokenType::RIGHT_BRACE)) {
            do {
                // Check for spread operator
                if (match({TokenType::DOT_DOT_DOT})) {
                    ExprPtr spreadArg = expression();
                    // Use empty string as key to indicate spread
                    entries.emplace_back("", std::make_unique<SpreadExpr>(std::move(spreadArg)));
                } else {
                    Token keyToken = consume(TokenType::STRING, "Expect string as map key.");
                    std::string key = std::string(keyToken.lexeme);
                    // Remove surrounding quotes
                    if (key.length() >= 2) {
                        key = key.substr(1, key.length() - 2);
                    }
                    consume(TokenType::COLON, "Expect ':' after map key.");
                    ExprPtr value = expression();
                    entries.emplace_back(std::move(key), std::move(value));
                }
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

// Pattern parsing for match expressions
PatternPtr Parser::parsePattern() {
    // Wildcard pattern: _
    if (match({TokenType::UNDERSCORE})) {
        return std::make_unique<WildcardPattern>();
    }
    
    // Literal patterns: numbers, strings, booleans, nil
    if (match({TokenType::NUMBER})) {
        double value = std::stod(std::string(previous().lexeme));
        return std::make_unique<LiteralPattern>(value);
    }
    
    if (match({TokenType::STRING})) {
        std::string value(previous().lexeme);
        // Remove surrounding quotes
        if (value.length() >= 2) {
            value = value.substr(1, value.length() - 2);
        }
        return std::make_unique<LiteralPattern>(value);
    }
    
    if (match({TokenType::TRUE})) {
        return std::make_unique<LiteralPattern>(true);
    }
    
    if (match({TokenType::FALSE})) {
        return std::make_unique<LiteralPattern>(false);
    }
    
    if (match({TokenType::NIL})) {
        return std::make_unique<LiteralPattern>(Nil{});
    }
    
    // Variable pattern: identifier
    if (match({TokenType::IDENTIFIER})) {
        Token name = previous();
        return std::make_unique<VariablePattern>(std::string(name.lexeme));
    }
    
    throw error(peek(), "Expect pattern in match expression.");
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

void Parser::consumeSemicolonIfNeeded() {
    // If there's a semicolon, consume it
    if (check(TokenType::SEMICOLON)) {
        advance();
        return;
    }
    
    // Semicolons are optional at end of line (newline) or end of file
    // They are required when multiple statements are on the same line
    if (isAtEnd()) {
        return;  // End of file - semicolon not needed
    }
    
    // Check if the next token is on a different line
    Token prev = previous();
    Token next = peek();
    
    // If next token is on a different line, semicolon is optional
    if (next.line > prev.line) {
        return;
    }
    
    // Multiple statements on same line without semicolon - error
    // Check if next token starts a new statement
    switch (next.type) {
        case TokenType::VAR:
        case TokenType::FN:
        case TokenType::CLASS:
        case TokenType::IF:
        case TokenType::WHILE:
        case TokenType::FOR:
        case TokenType::RETURN:
        case TokenType::BREAK:
        case TokenType::CONTINUE:
        case TokenType::PRINT:
        case TokenType::IMPORT:
        case TokenType::EXPORT:
        case TokenType::TRY:
        case TokenType::THROW:
        case TokenType::MATCH:
            throw error(next, "Expect ';' between statements on the same line.");
        default:
            // Other tokens might be part of the same expression or block delimiters
            break;
    }
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

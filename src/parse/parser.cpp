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
    int stmtLine = peek().line;
    try {
        StmtPtr stmt;
        if (match({TokenType::MACRO})) stmt = macroDeclaration();
        else if (match({TokenType::VAR})) stmt = varDeclaration();
        else if (check(TokenType::ASYNC) && current + 1 < tokens.size() &&
                 tokens[current + 1].type == TokenType::FN) {
            advance();  // consume 'async'
            advance();  // consume 'fn'
            stmt = functionDeclaration(true);
        }
        else if (match({TokenType::FN})) stmt = functionDeclaration();
        else if (match({TokenType::CLASS})) stmt = classDeclaration();
        else if (match({TokenType::IMPORT})) stmt = importStatement();
        else if (match({TokenType::EXPORT})) stmt = exportStatement();
        // Check for statement-level macro invocation: name!( ... )
        else if (check(TokenType::IDENTIFIER) && current + 1 < tokens.size() &&
            tokens[current + 1].type == TokenType::BANG && macros_.count(tokens[current].lexeme)) {
            stmt = expandMacroStmt();
        } else {
            stmt = statement();
        }
        if (stmt) stmt->line = stmtLine;
        return stmt;
    } catch (const std::runtime_error& e) {
        synchronize();
        return nullptr;
    }
}

StmtPtr Parser::varDeclaration() {
    // Check if this is a destructuring declaration
    if (check(TokenType::LEFT_BRACKET)) {
        // Array destructuring: var [a, b] = [1, 2];
        advance();  // consume '['

        std::vector<PatternPtr> elements;
        if (!check(TokenType::RIGHT_BRACKET)) {
            do {
                Token name = consume(TokenType::IDENTIFIER, "Expect variable name in array pattern.");
                elements.push_back(std::make_unique<VariablePattern>(std::string(name.lexeme)));
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACKET, "Expect ']' after array pattern.");

        ExprPtr initializer = nullptr;
        if (match({TokenType::EQUAL})) {
            initializer = expression();
        } else {
            throw error(peek(), "Array destructuring requires an initializer.");
        }

        consumeSemicolonIfNeeded();
        return std::make_unique<VarStmt>(std::make_unique<ArrayPattern>(std::move(elements)), std::move(initializer));
    }

    if (check(TokenType::LEFT_BRACE)) {
        // Map destructuring: var {name, age} = person;
        advance();  // consume '{'

        std::vector<std::string> keys;
        if (!check(TokenType::RIGHT_BRACE)) {
            do {
                Token key = consume(TokenType::IDENTIFIER, "Expect identifier in map pattern.");
                keys.push_back(std::string(key.lexeme));
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RIGHT_BRACE, "Expect '}' after map pattern.");

        ExprPtr initializer = nullptr;
        if (match({TokenType::EQUAL})) {
            initializer = expression();
        } else {
            throw error(peek(), "Map destructuring requires an initializer.");
        }

        consumeSemicolonIfNeeded();
        return std::make_unique<VarStmt>(std::make_unique<MapPattern>(std::move(keys)), std::move(initializer));
    }

    // Simple variable declaration
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

StmtPtr Parser::functionDeclaration(bool isAsync) {
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

    return std::make_unique<FunctionStmt>(std::string(name.lexeme), std::move(params), std::move(bodyStmts),
                                          std::move(paramTypes), std::move(returnType), isAsync);
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
        std::vector<std::string> namedAliases;
        bool hasAliases = false;
        do {
            // Allow 'default' keyword as an export name in import lists
            std::string exportName;
            if (match({TokenType::DEFAULT})) {
                exportName = "default";
            } else {
                Token name = consume(TokenType::IDENTIFIER, "Expect identifier in import list.");
                exportName = std::string(name.lexeme);
            }
            namedImports.push_back(exportName);
            // Optional alias: name as alias
            if (match({TokenType::AS})) {
                Token alias = consume(TokenType::IDENTIFIER, "Expect alias name after 'as'.");
                namedAliases.push_back(std::string(alias.lexeme));
                hasAliases = true;
            } else {
                namedAliases.push_back("");
            }
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

        if (hasAliases) {
            return std::make_unique<ImportStmt>(std::move(moduleName), std::move(namedImports),
                                                std::move(namedAliases));
        }
        return std::make_unique<ImportStmt>(std::move(moduleName), std::move(namedImports));
    }

    // Identifier import: import math;  (syntactic sugar for import "math";)
    if (check(TokenType::IDENTIFIER)) {
        Token nameToken = advance();
        consumeSemicolonIfNeeded();
        return std::make_unique<ImportStmt>(std::string(nameToken.lexeme));
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
    // export default <expr|fn|var>
    if (match({TokenType::DEFAULT})) {
        if (match({TokenType::FN})) {
            auto funcDecl = functionDeclaration();
            return std::make_unique<ExportStmt>(std::move(funcDecl), true);
        }
        if (match({TokenType::VAR})) {
            auto varDecl = varDeclaration();
            return std::make_unique<ExportStmt>(std::move(varDecl), true);
        }
        // export default <expr>;
        auto expr = expression();
        consumeSemicolonIfNeeded();
        return std::make_unique<ExportStmt>(std::move(expr));
    }

    // export * from "module";  (wildcard re-export)
    if (match({TokenType::STAR})) {
        consume(TokenType::FROM, "Expect 'from' after '*' in re-export statement.");
        Token moduleToken = consume(TokenType::STRING, "Expect module name as string.");
        consumeSemicolonIfNeeded();

        std::string moduleName = std::string(moduleToken.lexeme);
        if (moduleName.size() >= 2 && moduleName.front() == '"' && moduleName.back() == '"') {
            moduleName = moduleName.substr(1, moduleName.size() - 2);
        }
        return std::make_unique<ReExportStmt>(std::move(moduleName));
    }

    // export { a, b } from "module";  (named re-export)
    if (match({TokenType::LEFT_BRACE})) {
        std::vector<std::string> names;
        do {
            Token name = consume(TokenType::IDENTIFIER, "Expect identifier in export list.");
            names.push_back(std::string(name.lexeme));
        } while (match({TokenType::COMMA}));
        consume(TokenType::RIGHT_BRACE, "Expect '}' after export list.");
        consume(TokenType::FROM, "Expect 'from' after export list.");
        Token moduleToken = consume(TokenType::STRING, "Expect module name as string.");
        consumeSemicolonIfNeeded();

        std::string moduleName = std::string(moduleToken.lexeme);
        if (moduleName.size() >= 2 && moduleName.front() == '"' && moduleName.back() == '"') {
            moduleName = moduleName.substr(1, moduleName.size() - 2);
        }
        return std::make_unique<ReExportStmt>(std::move(moduleName), std::move(names));
    }

    // export fn name() { ... }  or  export async fn name() { ... }
    if (check(TokenType::ASYNC) && current + 1 < tokens.size() &&
        tokens[current + 1].type == TokenType::FN) {
        advance();  // consume 'async'
        advance();  // consume 'fn'
        auto funcDecl = functionDeclaration(true);
        return std::make_unique<ExportStmt>(std::move(funcDecl));
    }
    if (match({TokenType::FN})) {
        auto funcDecl = functionDeclaration();
        return std::make_unique<ExportStmt>(std::move(funcDecl));
    }

    // export var name = value;
    if (match({TokenType::VAR})) {
        auto varDecl = varDeclaration();
        return std::make_unique<ExportStmt>(std::move(varDecl));
    }

    throw error(peek(), "Expect 'fn', 'var', 'default', '*', or '{' after 'export'.");
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

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
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

    return std::make_unique<TryStmt>(std::move(tryBlock), std::move(catchVariable), std::move(catchBlock),
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
            fields.push_back(std::make_unique<VarStmt>(std::string(fieldName.lexeme), std::move(initializer),
                                                       std::move(typeAnnotation)));
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

            methods.push_back(std::make_unique<FunctionStmt>(std::string(methodName.lexeme), std::move(params),
                                                             std::move(bodyStmts), std::move(paramTypes),
                                                             std::move(returnType)));
        } else {
            throw error(peek(), "Expect 'var' or 'fn' in class body.");
        }
    }

    consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
    return std::make_unique<ClassStmt>(std::string(name.lexeme), std::move(superclass), std::move(fields),
                                       std::move(methods));
}

StmtPtr Parser::macroDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect macro name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after macro name.");

    std::vector<std::string> params;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            Token param = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            params.push_back(std::string(param.lexeme));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after macro parameters.");
    consume(TokenType::LEFT_BRACE, "Expect '{' before macro body.");

    // Capture all tokens until the matching closing '}'
    std::vector<Token> bodyTokens;
    int depth = 1;
    while (!isAtEnd() && depth > 0) {
        Token t = tokens[current];
        if (t.type == TokenType::LEFT_BRACE) {
            depth++;
        } else if (t.type == TokenType::RIGHT_BRACE) {
            depth--;
            if (depth == 0) {
                advance();  // consume closing '}'
                break;
            }
        }
        bodyTokens.push_back(t);
        advance();
    }

    if (depth != 0) {
        throw error(previous(), "Unterminated macro body.");
    }

    macros_[std::string(name.lexeme)] = MacroDefinition{std::move(params), std::move(bodyTokens)};
    // Macro definitions have no runtime representation
    return std::make_unique<BlockStmt>(std::vector<StmtPtr>{});
}

std::vector<std::vector<Token>> Parser::collectMacroArgs(size_t expectedCount, const Token& callToken) {
    std::vector<std::vector<Token>> args;

    if (!check(TokenType::RIGHT_PAREN)) {
        std::vector<Token> currentArg;
        int depth = 0;

        while (!isAtEnd()) {
            if (check(TokenType::RIGHT_PAREN) && depth == 0) break;
            if (check(TokenType::COMMA) && depth == 0) {
                args.push_back(std::move(currentArg));
                currentArg.clear();
                advance();  // consume ','
                continue;
            }
            Token t = tokens[current];
            if (t.type == TokenType::LEFT_PAREN || t.type == TokenType::LEFT_BRACKET ||
                t.type == TokenType::LEFT_BRACE) {
                depth++;
            } else if (t.type == TokenType::RIGHT_PAREN || t.type == TokenType::RIGHT_BRACKET ||
                       t.type == TokenType::RIGHT_BRACE) {
                depth--;
            }
            currentArg.push_back(t);
            advance();
        }
        if (!currentArg.empty()) args.push_back(std::move(currentArg));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after macro arguments.");

    if (args.size() != expectedCount) {
        throw error(callToken, "Macro '" + callToken.lexeme + "' expects " + std::to_string(expectedCount) +
                                   " argument(s), got " + std::to_string(args.size()) + ".");
    }
    return args;
}

std::vector<Token> Parser::substituteBody(const MacroDefinition& macro,
                                           const std::vector<std::vector<Token>>& args, int line, int col) {
    std::vector<Token> result;
    for (const auto& tok : macro.bodyTokens) {
        if (tok.type == TokenType::IDENTIFIER) {
            bool substituted = false;
            for (size_t i = 0; i < macro.params.size(); i++) {
                if (tok.lexeme == macro.params[i]) {
                    result.insert(result.end(), args[i].begin(), args[i].end());
                    substituted = true;
                    break;
                }
            }
            if (!substituted) result.push_back(tok);
        } else {
            result.push_back(tok);
        }
    }
    return result;
}

ExprPtr Parser::expandMacroExpr(const std::string& macroName) {
    auto& macro = macros_[macroName];
    Token bangToken = peek();
    advance();  // consume '!'
    consume(TokenType::LEFT_PAREN, "Expect '(' after '!' in macro invocation.");

    auto args = collectMacroArgs(macro.params.size(), bangToken);
    auto expanded = substituteBody(macro, args, bangToken.line, bangToken.column);

    // Insert expanded tokens at the current position in the token stream
    tokens.insert(tokens.begin() + static_cast<std::ptrdiff_t>(current), expanded.begin(), expanded.end());

    // Parse the now-inserted tokens as an expression
    return expression();
}

StmtPtr Parser::expandMacroStmt() {
    Token nameToken = tokens[current];
    advance();  // consume identifier
    advance();  // consume '!'
    consume(TokenType::LEFT_PAREN, "Expect '(' after '!' in macro invocation.");

    auto& macro = macros_[nameToken.lexeme];
    auto args = collectMacroArgs(macro.params.size(), nameToken);
    auto expanded = substituteBody(macro, args, nameToken.line, nameToken.column);

    // Wrap body in braces so it parses as a single block statement
    Token lbrace(TokenType::LEFT_BRACE, "{", nameToken.line, nameToken.column);
    Token rbrace(TokenType::RIGHT_BRACE, "}", nameToken.line, nameToken.column);

    std::vector<Token> wrapped;
    wrapped.push_back(lbrace);
    wrapped.insert(wrapped.end(), expanded.begin(), expanded.end());
    wrapped.push_back(rbrace);

    tokens.insert(tokens.begin() + static_cast<std::ptrdiff_t>(current), wrapped.begin(), wrapped.end());

    StmtPtr result = statement();  // parses the inserted '{...}' block
    consumeSemicolonIfNeeded();    // consume optional ';' after macro invocation
    return result;
}

// Type annotation parsing (v0.3)
TypePtr Parser::parseTypeAnnotation() {
    // Check for 'nil' keyword used as a type annotation
    if (match({TokenType::NIL})) {
        return TypeAnnotation::simple(TypeAnnotation::Kind::Nil);
    }

    // Check for basic types
    if (match({TokenType::IDENTIFIER})) {
        std::string typeName = std::string(previous().lexeme);

        // Check for generic types (Array<T>, Map<K,V>)
        if (match({TokenType::LESS})) {
            if (typeName == "Array" || typeName == "array") {
                TypePtr elementType = parseTypeAnnotation();
                consume(TokenType::GREATER, "Expect '>' after array element type.");
                return TypeAnnotation::array(std::move(elementType));
            } else if (typeName == "Map" || typeName == "map") {
                TypePtr keyType = parseTypeAnnotation();
                consume(TokenType::COMMA, "Expect ',' between map key and value types.");
                TypePtr valueType = parseTypeAnnotation();
                consume(TokenType::GREATER, "Expect '>' after map value type.");
                return TypeAnnotation::map(std::move(keyType), std::move(valueType));
            } else {
                throw error(previous(), "Unknown generic type '" + typeName + "'.");
            }
        }

        // Simple types (both capitalized and lowercase are accepted)
        if (typeName == "Number" || typeName == "number") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Number);
        } else if (typeName == "String" || typeName == "string") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::String);
        } else if (typeName == "Bool" || typeName == "bool") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Bool);
        } else if (typeName == "Nil") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Nil);
        } else if (typeName == "Any" || typeName == "any") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Any);
        } else if (typeName == "Void" || typeName == "void") {
            return TypeAnnotation::simple(TypeAnnotation::Kind::Void);
        } else {
            // User-defined type (e.g., class name) — treat as Any for gradual typing
            return TypeAnnotation::simple(TypeAnnotation::Kind::Any);
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
    ExprPtr expr = conditional();

    if (match({TokenType::EQUAL})) {
        const Token& equals = previous();
        ExprPtr value = assignment();

        // Variable assignment
        if (auto var = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(var->name, std::move(value));
        }
        // Index assignment: arr[i] = value
        if (auto indexExpr = dynamic_cast<IndexExpr*>(expr.get())) {
            return std::make_unique<SetIndexExpr>(std::move(indexExpr->collection), std::move(indexExpr->index),
                                                  std::move(value));
        }
        // Property assignment: obj.prop = value (v0.3)
        if (auto propExpr = dynamic_cast<PropertyExpr*>(expr.get())) {
            return std::make_unique<SetPropertyExpr>(std::move(propExpr->object), propExpr->property, std::move(value));
        }

        throw error(equals, "Invalid assignment target");
    }

    // Compound assignment operators: desugar x op= y into x = x op y
    if (match({TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL,
               TokenType::PERCENT_EQUAL})) {
        Token opEq = previous();

        // Map compound token to the underlying binary operator token
        TokenType binOp;
        switch (opEq.type) {
            case TokenType::PLUS_EQUAL:
                binOp = TokenType::PLUS;
                break;
            case TokenType::MINUS_EQUAL:
                binOp = TokenType::MINUS;
                break;
            case TokenType::STAR_EQUAL:
                binOp = TokenType::STAR;
                break;
            case TokenType::SLASH_EQUAL:
                binOp = TokenType::SLASH;
                break;
            case TokenType::PERCENT_EQUAL:
                binOp = TokenType::PERCENT;
                break;
            default:
                throw error(opEq, "Unknown compound assignment operator.");
        }
        Token op(binOp, opEq.lexeme.substr(0, opEq.lexeme.size() - 1), opEq.line, opEq.column);

        ExprPtr rhs = assignment();

        if (auto var = dynamic_cast<VariableExpr*>(expr.get())) {
            auto varRead = std::make_unique<VariableExpr>(var->name, nullptr);
            auto binary = std::make_unique<BinaryExpr>(std::move(varRead), op, std::move(rhs));
            return std::make_unique<AssignExpr>(var->name, std::move(binary));
        }

        throw error(opEq, "Compound assignment target must be a variable.");
    }

    return expr;
}

ExprPtr Parser::conditional() {
    ExprPtr expr = logicalOr();

    if (match({TokenType::QUESTION})) {
        ExprPtr thenBranch = expression();
        consume(TokenType::COLON, "Expect ':' after then branch of conditional expression.");
        ExprPtr elseBranch = conditional();  // Right-associative
        expr = std::make_unique<ConditionalExpr>(std::move(expr), std::move(thenBranch), std::move(elseBranch));
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

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
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

    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT})) {
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

    // await expression: await <expr>
    if (match({TokenType::AWAIT})) {
        ExprPtr value = unary();
        return std::make_unique<AwaitExpr>(std::move(value));
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

    // Function expression: fn(params) { body } or async fn(params) { body }
    bool asyncFn = false;
    if (check(TokenType::ASYNC) && current + 1 < tokens.size() &&
        tokens[current + 1].type == TokenType::FN) {
        advance();  // consume 'async'
        asyncFn = true;
    }
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

        return std::make_unique<FunctionExpr>(std::move(params), std::move(bodyStmts), asyncFn);
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
        // Check for expression-level macro invocation: name!( ... )
        if (check(TokenType::BANG) && macros_.count(name.lexeme)) {
            return expandMacroExpr(std::string(name.lexeme));
        }
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

    if (match({TokenType::LEFT_BRACE})) {
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

#include "lexer.hpp"

#include <cctype>
#include <stdexcept>

namespace izi {

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", line, column);
    return tokens;
}

void Lexer::scanToken() {
    char c = advance();

    switch (c) {
        case '(':
            addToken(TokenType::LEFT_PAREN);
            break;
        case ')':
            addToken(TokenType::RIGHT_PAREN);
            break;
        case '{':
            addToken(TokenType::LEFT_BRACE);
            break;
        case '}':
            addToken(TokenType::RIGHT_BRACE);
            break;
        case '[':
            addToken(TokenType::LEFT_BRACKET);
            break;
        case ']':
            addToken(TokenType::RIGHT_BRACKET);
            break;
        case ',':
            addToken(TokenType::COMMA);
            break;
        case '.':
            addToken(TokenType::DOT);
            break;
        case ';':
            addToken(TokenType::SEMICOLON);
            break;
        case ':':
            addToken(TokenType::COLON);
            break;
        case '+':
            addToken(TokenType::PLUS);
            break;
        case '*':
            addToken(TokenType::STAR);
            break;
        case '/':
            addToken(TokenType::SLASH);
            break;
        case '-':
            addToken(match('>') ? TokenType::ARROW : TokenType::MINUS);
            break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line++;
            break;
        case '"':
            string();
            break;
        default:
            if (isdigit(c)) {
                number();
            } else if (isalpha(c) || c == '_') {
                identifier();
            } else {
                throw std::runtime_error("Unexpected character at line " + std::to_string(line));
            }
            break;
    }
}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

char Lexer::advance() {
    return source[current++];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    current++;
    return true;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

void Lexer::addToken(TokenType type) {
    std::string lexeme = source.substr(start, current - start);
    tokens.emplace_back(type, lexeme, line, column);
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        advance();
    }

    if (isAtEnd()) {
        throw std::runtime_error("Unterminated string at line " + std::to_string(line));
    }

    advance();  // Closing "

    std::string lexeme = source.substr(start, current - start);
    tokens.emplace_back(TokenType::STRING, lexeme, line, column);
}

void Lexer::number() {
    while (isdigit(peek())) advance();

    if (peek() == '.' && isdigit(peekNext())) {
        advance();  // Consume '.'
        while (isdigit(peek())) advance();
    }

    std::string_view lexeme = source.substr(start, current - start);
    tokens.emplace_back(TokenType::NUMBER, lexeme, line, column);
}

void Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_') advance();

    std::string_view lexeme = source.substr(start, current - start);
    TokenType type = keywordType(lexeme);
    tokens.emplace_back(type, lexeme, line, column);
}

TokenType Lexer::keywordType(std::string_view text) {
    if (text == "fn") return TokenType::FN;
    if (text == "var") return TokenType::VAR;
    if (text == "if") return TokenType::IF;
    if (text == "else") return TokenType::ELSE;
    if (text == "while") return TokenType::WHILE;
    if (text == "for") return TokenType::FOR;
    if (text == "return") return TokenType::RETURN;
    if (text == "break") return TokenType::BREAK;
    if (text == "continue") return TokenType::CONTINUE;
    if (text == "class") return TokenType::CLASS;
    if (text == "true") return TokenType::TRUE;
    if (text == "false") return TokenType::FALSE;
    if (text == "nil") return TokenType::NIL;
    return TokenType::IDENTIFIER;
}
}  // namespace izi

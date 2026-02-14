#include "lexer.hpp"

#include <cctype>
#include <stdexcept>

namespace izi {

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        startLine = line;
        startColumn = column;
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
            // Check for ... (spread operator)
            if (match('.') && match('.')) {
                addToken(TokenType::DOT_DOT_DOT);
            } else {
                addToken(TokenType::DOT);
            }
            break;
        case ';':
            addToken(TokenType::SEMICOLON);
            break;
        case ':':
            addToken(TokenType::COLON);
            break;
        case '?':
            addToken(TokenType::QUESTION);
            break;
        case '+':
            addToken(TokenType::PLUS);
            break;
        case '*':
            addToken(TokenType::STAR);
            break;
        case '/':
            if (match('/')) {
                skipLineComment();
            } else if (match('*')) {
                skipBlockComment();
            } else {
                addToken(TokenType::SLASH);
            }
            break;
        case '-':
            addToken(match('>') ? TokenType::ARROW : TokenType::MINUS);
            break;
        case '!':
            addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG);
            break;
        case '=':
            if (match('=')) {
                addToken(TokenType::EQUAL_EQUAL);
            } else if (match('>')) {
                addToken(TokenType::ARROW);  // => for match expressions
            } else {
                addToken(TokenType::EQUAL);
            }
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
        case '\n':
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
                throw LexerError(startLine, startColumn, "Unexpected character '" + std::string(1, c) + "'");
            }
            break;
    }
}

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

char Lexer::advance() {
    char c = source[current++];
    column++;
    if (c == '\n') {
        line++;
        column = 0;
    }
    return c;
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
    tokens.emplace_back(type, lexeme, startLine, startColumn);
}

void Lexer::string() {
    int stringStart = start;
    int stringStartLine = startLine;
    int stringStartColumn = startColumn;
    
    // Track parts of the interpolated string
    bool hasInterpolation = false;
    bool emittedTokenForThisString = false;  // Track if we've emitted any tokens for this string
    int partStart = current;  // Start of current string part (after opening quote)
    
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') line++;
        
        // Check for interpolation: ${
        if (peek() == '$' && peekNext() == '{') {
            hasInterpolation = true;
            
            // Emit the string part before the interpolation
            if (current > partStart) {
                std::string strPart = "\"" + source.substr(partStart, current - partStart) + "\"";
                tokens.emplace_back(TokenType::STRING, strPart, stringStartLine, stringStartColumn);
                
                // Emit PLUS for concatenation
                tokens.emplace_back(TokenType::PLUS, "+", line, column);
                emittedTokenForThisString = true;
            } else if (!emittedTokenForThisString) {
                // Empty string at the beginning of this specific string - no token needed yet
            } else {
                // Empty string in the middle, emit empty string
                tokens.emplace_back(TokenType::STRING, "\"\"", stringStartLine, stringStartColumn);
                tokens.emplace_back(TokenType::PLUS, "+", line, column);
            }
            
            // Skip ${ 
            advance();  // $
            advance();  // {
            
            // Emit str( to wrap the expression
            tokens.emplace_back(TokenType::IDENTIFIER, "str", line, column);
            tokens.emplace_back(TokenType::LEFT_PAREN, "(", line, column);
            emittedTokenForThisString = true;
            
            // Tokenize the expression inside ${}
            int braceDepth = 1;
            while (braceDepth > 0 && !isAtEnd()) {
                if (peek() == '{') {
                    braceDepth++;
                } else if (peek() == '}') {
                    braceDepth--;
                    if (braceDepth == 0) {
                        advance();  // Consume closing }
                        break;
                    }
                }
                
                // Scan the token normally
                start = current;
                startLine = line;
                startColumn = column;
                scanToken();
            }
            
            if (braceDepth != 0) {
                throw LexerError(stringStartLine, stringStartColumn, "Unterminated interpolation in string");
            }
            
            // Emit ) to close str() call
            tokens.emplace_back(TokenType::RIGHT_PAREN, ")", line, column);
            
            // Emit PLUS for concatenation with next part
            tokens.emplace_back(TokenType::PLUS, "+", line, column);
            
            // Update partStart to continue scanning the rest of the string
            partStart = current;
            continue;
        }
        
        advance();
    }

    if (isAtEnd()) {
        throw LexerError(stringStartLine, stringStartColumn, "Unterminated string");
    }

    advance();  // Closing "

    if (hasInterpolation) {
        // Emit the final string part
        if (current - 1 > partStart) {
            std::string strPart = "\"" + source.substr(partStart, current - 1 - partStart) + "\"";
            tokens.emplace_back(TokenType::STRING, strPart, line, column);
        } else {
            // Empty string at the end
            tokens.emplace_back(TokenType::STRING, "\"\"", line, column);
        }
    } else {
        // No interpolation, emit the whole string as before
        std::string lexeme = source.substr(stringStart, current - stringStart);
        tokens.emplace_back(TokenType::STRING, lexeme, stringStartLine, stringStartColumn);
    }
}

void Lexer::number() {
    while (isdigit(peek())) advance();

    if (peek() == '.' && isdigit(peekNext())) {
        advance();  // Consume '.'
        while (isdigit(peek())) advance();
    }

    std::string_view lexeme = source.substr(start, current - start);
    tokens.emplace_back(TokenType::NUMBER, lexeme, startLine, startColumn);
}

void Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_') advance();

    std::string_view lexeme = source.substr(start, current - start);
    
    // Check for standalone underscore (wildcard pattern)
    if (lexeme == "_") {
        tokens.emplace_back(TokenType::UNDERSCORE, lexeme, startLine, startColumn);
        return;
    }
    
    TokenType type = keywordType(lexeme);
    tokens.emplace_back(type, lexeme, startLine, startColumn);
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
    if (text == "extends") return TokenType::EXTENDS;
    if (text == "this") return TokenType::THIS;
    if (text == "super") return TokenType::SUPER;
    if (text == "true") return TokenType::TRUE;
    if (text == "false") return TokenType::FALSE;
    if (text == "nil") return TokenType::NIL;
    if (text == "and") return TokenType::AND;
    if (text == "or") return TokenType::OR;
    if (text == "import") return TokenType::IMPORT;
    if (text == "export") return TokenType::EXPORT;
    if (text == "from") return TokenType::FROM;
    if (text == "as") return TokenType::AS;
    if (text == "try") return TokenType::TRY;
    if (text == "catch") return TokenType::CATCH;
    if (text == "finally") return TokenType::FINALLY;
    if (text == "throw") return TokenType::THROW;
    if (text == "match") return TokenType::MATCH;
    return TokenType::IDENTIFIER;
}

void Lexer::skipLineComment() {
    // Skip until end of line or end of file
    while (peek() != '\n' && !isAtEnd()) {
        advance();
    }
}

void Lexer::skipBlockComment() {
    // Use the start position captured before we began scanning this token
    // This points to the '/' at the beginning of '/*'
    int commentStartLine = startLine;
    int commentStartColumn = startColumn;
    
    // Skip until we find */ or reach end of file
    while (!isAtEnd()) {
        if (peek() == '*' && peekNext() == '/') {
            advance();  // Consume *
            advance();  // Consume /
            return;
        }
        advance();
    }
    
    // If we reach here, we have an unterminated comment
    throw LexerError(commentStartLine, commentStartColumn, "Unterminated block comment");
}
}  // namespace izi

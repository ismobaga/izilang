#include "error_reporter.hpp"
#include <sstream>
#include <algorithm>

namespace izi {

void ErrorReporter::splitLines() {
    size_t start = 0;
    size_t pos = 0;

    while (pos < source_.size()) {
        if (source_[pos] == '\n') {
            lines_.push_back(source_.substr(start, pos - start));
            start = pos + 1;
        }
        pos++;
    }

    // Add the last line if it doesn't end with newline
    if (start < source_.size()) {
        lines_.push_back(source_.substr(start));
    }
}

std::string ErrorReporter::getLineContent(int line) const {
    if (line < 1 || static_cast<size_t>(line) > lines_.size()) {
        return "";
    }
    return std::string(lines_[line - 1]);
}

std::string ErrorReporter::createUnderline(int column, int length) const {
    if (column < 1) column = 1;
    if (length < 1) length = 1;

    std::string underline;
    underline.append(column - 1, ' ');
    underline.append(length, '^');
    return underline;
}

std::string ErrorReporter::colorize(const std::string& text, const char* color) const {
    if (!useColor_) {
        return text;
    }
    return std::string(color) + text + Color::RESET;
}

std::string ErrorReporter::formatError(const Token& token, const std::string& message,
                                       const std::string& errorType) const {
    std::ostringstream oss;

    // Error header
    oss << colorize(errorType, Color::RED) << colorize(" at line ", Color::BOLD) << token.line
        << colorize(", column ", Color::BOLD) << token.column << ":\n";

    // Get the line content
    std::string lineContent = getLineContent(token.line);

    if (!lineContent.empty()) {
        // Line number prefix
        std::string lineNumStr = std::to_string(token.line);
        oss << "  " << colorize(lineNumStr, Color::CYAN) << " | " << lineContent << "\n";

        // Underline the error position using token's lexeme length
        int length = token.lexeme.length() > 0 ? token.lexeme.length() : 1;

        std::string underline = createUnderline(token.column, length);
        oss << "  " << std::string(lineNumStr.length(), ' ') << " | " << colorize(underline, Color::RED) << "\n";
    }

    // Error message
    oss << colorize(message, Color::BOLD);

    return oss.str();
}

std::string ErrorReporter::formatError(int line, int column, const std::string& message,
                                       const std::string& errorType) const {
    std::ostringstream oss;

    // Error header
    oss << colorize(errorType, Color::RED) << colorize(" at line ", Color::BOLD) << line
        << colorize(", column ", Color::BOLD) << column << ":\n";

    // Get the line content
    std::string lineContent = getLineContent(line);

    if (!lineContent.empty()) {
        // Line number prefix
        std::string lineNumStr = std::to_string(line);
        oss << "  " << colorize(lineNumStr, Color::CYAN) << " | " << lineContent << "\n";

        // Underline the error position
        // Calculate the length based on the token lexeme if available
        int length = 1;  // Default to single character

        std::string underline = createUnderline(column, length);
        oss << "  " << std::string(lineNumStr.length(), ' ') << " | " << colorize(underline, Color::RED) << "\n";
    }

    // Error message
    oss << colorize(message, Color::BOLD);

    return oss.str();
}

}  // namespace izi

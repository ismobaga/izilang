#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "token.hpp"

namespace izi {

// ANSI color codes for terminal output
namespace Color {
constexpr const char* RESET = "\033[0m";
constexpr const char* RED = "\033[31m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* CYAN = "\033[36m";
constexpr const char* BOLD = "\033[1m";
}  // namespace Color

class ErrorReporter {
   public:
    explicit ErrorReporter(std::string_view source, bool useColor = true) : source_(source), useColor_(useColor) {
        splitLines();
    }

    // Format a runtime error with source context
    std::string formatError(const Token& token, const std::string& message,
                            const std::string& errorType = "Runtime Error") const;

    // Format a parser/lexer error with source context
    std::string formatError(int line, int column, const std::string& message,
                            const std::string& errorType = "Error") const;

   private:
    std::string_view source_;
    std::vector<std::string_view> lines_;
    bool useColor_;

    void splitLines();
    std::string getLineContent(int line) const;
    std::string createUnderline(int column, int length) const;
    std::string colorize(const std::string& text, const char* color) const;
};

}  // namespace izi

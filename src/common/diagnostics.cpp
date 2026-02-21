#include "diagnostics.hpp"

#include <algorithm>
#include <sstream>

namespace izi {

// ANSI color codes
namespace DiagColor {
constexpr const char* RESET = "\033[0m";
constexpr const char* RED = "\033[31m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* CYAN = "\033[36m";
constexpr const char* BLUE = "\033[34m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* BOLD = "\033[1m";
}  // namespace DiagColor

// ---- DiagnosticEngine -------------------------------------------------------

void DiagnosticEngine::splitLines() {
    size_t start = 0;
    for (size_t i = 0; i < source_.size(); ++i) {
        if (source_[i] == '\n') {
            lines_.push_back(source_.substr(start, i - start));
            start = i + 1;
        }
    }
    if (start < source_.size()) {
        lines_.push_back(source_.substr(start));
    }
}

std::string DiagnosticEngine::getLineContent(int line) const {
    if (line < 1 || static_cast<size_t>(line) > lines_.size()) {
        return "";
    }
    return std::string(lines_[line - 1]);
}

std::string DiagnosticEngine::createUnderline(int column, int length, char underlineChar) const {
    if (column < 1) column = 1;
    if (length < 1) length = 1;
    std::string s;
    s.append(static_cast<size_t>(column - 1), ' ');
    s.append(static_cast<size_t>(length), underlineChar);
    return s;
}

std::string DiagnosticEngine::colorize(const std::string& text, const char* color) const {
    if (!useColor_) return text;
    return std::string(color) + text + DiagColor::RESET;
}

const char* DiagnosticEngine::levelLabel(DiagnosticLevel level) {
    switch (level) {
        case DiagnosticLevel::Error:
            return "error";
        case DiagnosticLevel::Warning:
            return "warning";
        case DiagnosticLevel::Note:
            return "note";
        case DiagnosticLevel::Help:
            return "help";
    }
    return "error";
}

const char* DiagnosticEngine::levelColor(DiagnosticLevel level) {
    switch (level) {
        case DiagnosticLevel::Error:
            return DiagColor::RED;
        case DiagnosticLevel::Warning:
            return DiagColor::YELLOW;
        case DiagnosticLevel::Note:
            return DiagColor::CYAN;
        case DiagnosticLevel::Help:
            return DiagColor::GREEN;
    }
    return DiagColor::RED;
}

char DiagnosticEngine::levelUnderlineChar(DiagnosticLevel level) {
    switch (level) {
        case DiagnosticLevel::Error:
            return '^';
        case DiagnosticLevel::Warning:
            return '~';
        default:
            return '-';
    }
}

void DiagnosticEngine::report(Diagnostic diag) {
    if (diag.level == DiagnosticLevel::Error) {
        ++errorCount_;
    } else if (diag.level == DiagnosticLevel::Warning) {
        ++warningCount_;
    }
    diagnostics_.push_back(std::move(diag));
}

void DiagnosticEngine::error(std::string code, std::string message, SourceRange range) {
    report(Diagnostic(DiagnosticLevel::Error, std::move(code), std::move(message), range));
}

void DiagnosticEngine::warning(std::string code, std::string message, SourceRange range) {
    report(Diagnostic(DiagnosticLevel::Warning, std::move(code), std::move(message), range));
}

void DiagnosticEngine::note(std::string message, SourceRange range) {
    report(Diagnostic(DiagnosticLevel::Note, "", std::move(message), range));
}

std::string DiagnosticEngine::format(const Diagnostic& diag) const {
    std::ostringstream oss;

    const char* color = levelColor(diag.level);
    const char* label = levelLabel(diag.level);

    // Header: "error[E001]: message"
    std::string header = std::string(label);
    if (!diag.code.empty()) {
        header += "[" + diag.code + "]";
    }
    header += ": " + diag.message;
    oss << colorize(header, color) << "\n";

    // Location line: " --> line:column"
    int line = diag.range.begin.line;
    int col = diag.range.begin.column;
    int len = diag.range.begin.length;
    if (line > 0) {
        oss << colorize(" --> ", DiagColor::CYAN) << line << ":" << col << "\n";

        std::string lineContent = getLineContent(line);
        std::string lineNumStr = std::to_string(line);
        std::string gutterPad(lineNumStr.size(), ' ');

        // Blank gutter line
        oss << " " << gutterPad << colorize(" |", DiagColor::CYAN) << "\n";

        // Source line
        oss << " " << colorize(lineNumStr, DiagColor::CYAN) << colorize(" | ", DiagColor::CYAN) << lineContent
            << "\n";

        // Underline
        char underlineChar = levelUnderlineChar(diag.level);
        std::string underline = createUnderline(col, len, underlineChar);
        oss << " " << gutterPad << colorize(" | ", DiagColor::CYAN) << colorize(underline, color) << "\n";
    }

    // Notes / help
    for (const auto& note : diag.notes) {
        const char* noteColor = levelColor(note.level);
        const char* noteLabel = levelLabel(note.level);

        // If there's a source range, show it
        if (note.range.has_value()) {
            int nline = note.range->begin.line;
            int ncol = note.range->begin.column;
            int nlen = note.range->begin.length;
            std::string nlineContent = getLineContent(nline);
            std::string nlineNumStr = std::to_string(nline);
            std::string ngutterPad(nlineNumStr.size(), ' ');

            oss << colorize(std::string(noteLabel) + ": " + note.message, noteColor) << "\n";
            oss << colorize(" --> ", DiagColor::CYAN) << nline << ":" << ncol << "\n";
            oss << " " << ngutterPad << colorize(" |", DiagColor::CYAN) << "\n";
            oss << " " << colorize(nlineNumStr, DiagColor::CYAN) << colorize(" | ", DiagColor::CYAN) << nlineContent
                << "\n";
            char underlineChar = levelUnderlineChar(note.level);
            std::string underline = createUnderline(ncol, nlen, underlineChar);
            oss << " " << ngutterPad << colorize(" | ", DiagColor::CYAN) << colorize(underline, noteColor) << "\n";
        } else {
            // Inline note without range: "  = note: message"
            oss << colorize("  = " + std::string(noteLabel) + ": ", noteColor) << note.message << "\n";
        }
    }

    return oss.str();
}

std::string DiagnosticEngine::formatAll() const {
    std::ostringstream oss;
    for (const auto& diag : diagnostics_) {
        oss << format(diag);
        if (&diag != &diagnostics_.back()) {
            oss << "\n";
        }
    }
    return oss.str();
}

}  // namespace izi

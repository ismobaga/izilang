#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace izi {

// Severity level of a diagnostic message
enum class DiagnosticLevel {
    Error,
    Warning,
    Note,
    Help,
};

// A position in source code
struct SourceLocation {
    int line;
    int column;
    int length;  // number of characters this location spans

    SourceLocation(int line = 0, int column = 0, int length = 1)
        : line(line), column(column), length(length) {}
};

// A span of source code from begin to end
struct SourceRange {
    SourceLocation begin;
    SourceLocation end;

    SourceRange() = default;

    // Single-location range (begin == end)
    explicit SourceRange(SourceLocation loc) : begin(loc), end(loc) {}

    SourceRange(SourceLocation begin, SourceLocation end) : begin(begin), end(end) {}

    // Convenience: construct from line/column/length
    SourceRange(int line, int column, int length = 1)
        : begin(line, column, length), end(line, column, length) {}
};

// An attached note or help message for a diagnostic
struct DiagnosticNote {
    DiagnosticLevel level;  // Note or Help
    std::string message;
    std::optional<SourceRange> range;

    explicit DiagnosticNote(std::string msg, DiagnosticLevel lvl = DiagnosticLevel::Note)
        : level(lvl), message(std::move(msg)) {}

    DiagnosticNote(std::string msg, SourceRange r, DiagnosticLevel lvl = DiagnosticLevel::Note)
        : level(lvl), message(std::move(msg)), range(std::move(r)) {}
};

// A single diagnostic (error, warning, note) with source location and optional notes
struct Diagnostic {
    DiagnosticLevel level;
    std::string code;     // e.g. "E001", "W001"
    std::string message;
    SourceRange range;
    std::vector<DiagnosticNote> notes;

    Diagnostic(DiagnosticLevel level, std::string code, std::string message, SourceRange range)
        : level(level), code(std::move(code)), message(std::move(message)), range(range) {}

    // Add a note (informational attachment)
    Diagnostic& addNote(std::string msg) {
        notes.emplace_back(std::move(msg), DiagnosticLevel::Note);
        return *this;
    }

    // Add a note with a source range
    Diagnostic& addNote(std::string msg, SourceRange r) {
        notes.emplace_back(std::move(msg), std::move(r), DiagnosticLevel::Note);
        return *this;
    }

    // Add a help message
    Diagnostic& addHelp(std::string msg) {
        notes.emplace_back(std::move(msg), DiagnosticLevel::Help);
        return *this;
    }
};

// Collects diagnostics and formats them with source context
class DiagnosticEngine {
   public:
    explicit DiagnosticEngine(std::string_view source, bool useColor = true)
        : source_(source), useColor_(useColor) {
        splitLines();
    }

    // Report a pre-built diagnostic
    void report(Diagnostic diag);

    // Convenience reporters
    void error(std::string code, std::string message, SourceRange range);
    void warning(std::string code, std::string message, SourceRange range);
    void note(std::string message, SourceRange range);

    bool hasErrors() const { return errorCount_ > 0; }
    bool hasWarnings() const { return warningCount_ > 0; }
    size_t errorCount() const { return errorCount_; }
    size_t warningCount() const { return warningCount_; }

    const std::vector<Diagnostic>& diagnostics() const { return diagnostics_; }

    // Format a single diagnostic with source context
    std::string format(const Diagnostic& diag) const;

    // Format all collected diagnostics
    std::string formatAll() const;

   private:
    std::string_view source_;
    std::vector<std::string_view> lines_;
    bool useColor_;
    std::vector<Diagnostic> diagnostics_;
    size_t errorCount_ = 0;
    size_t warningCount_ = 0;

    void splitLines();
    std::string getLineContent(int line) const;
    std::string createUnderline(int column, int length, char underlineChar) const;
    std::string colorize(const std::string& text, const char* color) const;

    static const char* levelLabel(DiagnosticLevel level);
    static const char* levelColor(DiagnosticLevel level);
    static char levelUnderlineChar(DiagnosticLevel level);
};

}  // namespace izi

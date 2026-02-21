#include "catch.hpp"

#include "common/diagnostics.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"

using namespace izi;

// Helper: strip ANSI color codes for snapshot comparisons
static std::string stripAnsi(std::string s) {
    std::string out;
    bool inEscape = false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\033') {
            inEscape = true;
        } else if (inEscape) {
            if (s[i] == 'm') inEscape = false;
        } else {
            out += s[i];
        }
    }
    return out;
}

// ---- DiagnosticEngine construction -----------------------------------------

TEST_CASE("DiagnosticEngine: basic construction", "[diagnostics]") {
    SECTION("Starts with zero errors and warnings") {
        DiagnosticEngine engine("var x = 1;");
        REQUIRE(engine.errorCount() == 0);
        REQUIRE(engine.warningCount() == 0);
        REQUIRE(engine.diagnostics().empty());
    }
}

// ---- Severity levels --------------------------------------------------------

TEST_CASE("DiagnosticEngine: severity levels", "[diagnostics]") {
    SECTION("Error is counted as error") {
        DiagnosticEngine engine("var x = @;", false);
        engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));
        REQUIRE(engine.hasErrors());
        REQUIRE(engine.errorCount() == 1);
        REQUIRE(engine.warningCount() == 0);
    }

    SECTION("Warning is counted as warning") {
        DiagnosticEngine engine("var x = 1;", false);
        engine.warning("W001", "unused variable 'x'", SourceRange(1, 5, 1));
        REQUIRE(!engine.hasErrors());
        REQUIRE(engine.hasWarnings());
        REQUIRE(engine.warningCount() == 1);
    }

    SECTION("Note does not count as error or warning") {
        DiagnosticEngine engine("var x = 1;", false);
        engine.note("consider using a constant", SourceRange(1, 1, 3));
        REQUIRE(!engine.hasErrors());
        REQUIRE(!engine.hasWarnings());
    }

    SECTION("Multiple diagnostics accumulate") {
        DiagnosticEngine engine("var x = @;\nvar y = #;", false);
        engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));
        engine.error("E001", "unexpected character '#'", SourceRange(2, 9, 1));
        REQUIRE(engine.errorCount() == 2);
        REQUIRE(engine.diagnostics().size() == 2);
    }
}

// ---- Source range display ---------------------------------------------------

TEST_CASE("DiagnosticEngine: source range in formatted output", "[diagnostics]") {
    SECTION("Error header contains code and message") {
        DiagnosticEngine engine("var x = @;", false);
        engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));
        const auto& diag = engine.diagnostics().front();
        std::string out = stripAnsi(engine.format(diag));

        REQUIRE(out.find("error[E001]") != std::string::npos);
        REQUIRE(out.find("unexpected character '@'") != std::string::npos);
    }

    SECTION("Formatted output contains location arrow") {
        DiagnosticEngine engine("var x = @;", false);
        engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));
        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

        REQUIRE(out.find("--> 1:9") != std::string::npos);
    }

    SECTION("Formatted output shows source line") {
        DiagnosticEngine engine("var x = @;", false);
        engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));
        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

        REQUIRE(out.find("var x = @;") != std::string::npos);
    }

    SECTION("Formatted output has underline at correct column") {
        DiagnosticEngine engine("var x = @;", false);
        engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));
        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

        // The underline should be 8 spaces then '^' (column 9)
        REQUIRE(out.find("        ^") != std::string::npos);
    }

    SECTION("Multi-character underline spans the token length") {
        DiagnosticEngine engine("var myLongName = 1;", false);
        engine.error("E100", "undefined variable", SourceRange(1, 5, 10));
        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

        // 4 spaces then 10 carets
        REQUIRE(out.find("    ^^^^^^^^^^") != std::string::npos);
    }

    SECTION("Warning uses tilde underline") {
        DiagnosticEngine engine("var x = 1;", false);
        engine.warning("W001", "unused variable 'x'", SourceRange(1, 5, 1));
        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

        REQUIRE(out.find("warning[W001]") != std::string::npos);
        REQUIRE(out.find("~") != std::string::npos);
    }
}

// ---- Notes and help ---------------------------------------------------------

TEST_CASE("DiagnosticEngine: notes and help attached to diagnostics", "[diagnostics]") {
    SECTION("Note without source range uses inline format") {
        DiagnosticEngine engine("var x = @;", false);
        Diagnostic d(DiagnosticLevel::Error, "E001", "unexpected character '@'",
                     SourceRange(1, 9, 1));
        d.addNote("only ASCII printable characters are allowed");
        engine.report(d);

        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));
        REQUIRE(out.find("= note: only ASCII printable characters are allowed") != std::string::npos);
    }

    SECTION("Help without source range uses inline format") {
        DiagnosticEngine engine("var x = 1;", false);
        Diagnostic d(DiagnosticLevel::Error, "E002", "unterminated string",
                     SourceRange(1, 9, 1));
        d.addHelp("add a closing `\"` to end the string");
        engine.report(d);

        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));
        REQUIRE(out.find("= help: add a closing") != std::string::npos);
    }

    SECTION("Note with source range shows source context") {
        std::string src = "var x = 1;\nvar y = x + z;";
        DiagnosticEngine engine(src, false);
        Diagnostic d(DiagnosticLevel::Error, "E100", "undefined variable 'z'",
                     SourceRange(2, 13, 1));
        d.addNote("'x' was declared here", SourceRange(1, 5, 1));
        engine.report(d);

        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));
        REQUIRE(out.find("note: 'x' was declared here") != std::string::npos);
        REQUIRE(out.find("--> 1:5") != std::string::npos);
    }

    SECTION("Multiple notes are all rendered") {
        DiagnosticEngine engine("var x = 1;", false);
        Diagnostic d(DiagnosticLevel::Warning, "W001", "unused variable",
                     SourceRange(1, 5, 1));
        d.addNote("variable declared but never read");
        d.addHelp("remove it or prefix with `_` to suppress");
        engine.report(d);

        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));
        REQUIRE(out.find("= note: variable declared but never read") != std::string::npos);
        REQUIRE(out.find("= help: remove it or prefix") != std::string::npos);
    }
}

// ---- formatAll --------------------------------------------------------------

TEST_CASE("DiagnosticEngine: formatAll", "[diagnostics]") {
    SECTION("Returns empty string when no diagnostics") {
        DiagnosticEngine engine("var x = 1;", false);
        REQUIRE(engine.formatAll().empty());
    }

    SECTION("Formats multiple diagnostics separated by blank lines") {
        DiagnosticEngine engine("var x = @;\nvar y = #;", false);
        engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));
        engine.error("E001", "unexpected character '#'", SourceRange(2, 9, 1));

        std::string out = stripAnsi(engine.formatAll());
        REQUIRE(out.find("unexpected character '@'") != std::string::npos);
        REQUIRE(out.find("unexpected character '#'") != std::string::npos);
    }
}

// ---- Snapshot tests: exact formatted output ---------------------------------

TEST_CASE("DiagnosticEngine: snapshot - lexer unexpected character", "[diagnostics][snapshot]") {
    // Source with an illegal '@' character
    const std::string src = "var x = @;";
    DiagnosticEngine engine(src, false);
    engine.error("E001", "unexpected character '@'", SourceRange(1, 9, 1));

    std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

    // Verify key structural parts
    CHECK(out.find("error[E001]: unexpected character '@'") != std::string::npos);
    CHECK(out.find("--> 1:9") != std::string::npos);
    CHECK(out.find("var x = @;") != std::string::npos);
    CHECK(out.find("        ^") != std::string::npos);
}

TEST_CASE("DiagnosticEngine: snapshot - unterminated string with help", "[diagnostics][snapshot]") {
    const std::string src = "var s = \"hello";
    DiagnosticEngine engine(src, false);
    Diagnostic d(DiagnosticLevel::Error, "E002", "unterminated string",
                 SourceRange(1, 9, 1));
    d.addHelp("add a closing `\"` to end the string");
    engine.report(d);

    std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

    CHECK(out.find("error[E002]: unterminated string") != std::string::npos);
    CHECK(out.find("--> 1:9") != std::string::npos);
    CHECK(out.find("= help: add a closing") != std::string::npos);
}

TEST_CASE("DiagnosticEngine: snapshot - unterminated block comment", "[diagnostics][snapshot]") {
    const std::string src = "/* comment without end";
    DiagnosticEngine engine(src, false);
    Diagnostic d(DiagnosticLevel::Error, "E003", "unterminated block comment",
                 SourceRange(1, 1, 2));
    d.addHelp("add `*/` to close the comment");
    engine.report(d);

    std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

    CHECK(out.find("error[E003]: unterminated block comment") != std::string::npos);
    CHECK(out.find("--> 1:1") != std::string::npos);
    CHECK(out.find("= help: add `*/` to close the comment") != std::string::npos);
}

TEST_CASE("DiagnosticEngine: snapshot - parser error", "[diagnostics][snapshot]") {
    const std::string src = "var x = ;";
    DiagnosticEngine engine(src, false);
    engine.error("E100", "expected expression", SourceRange(1, 9, 1));

    std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

    CHECK(out.find("error[E100]: expected expression") != std::string::npos);
    CHECK(out.find("--> 1:9") != std::string::npos);
    CHECK(out.find("var x = ;") != std::string::npos);
}

TEST_CASE("DiagnosticEngine: snapshot - warning with note", "[diagnostics][snapshot]") {
    const std::string src = "var unused = 42;";
    DiagnosticEngine engine(src, false);
    Diagnostic d(DiagnosticLevel::Warning, "W001", "unused variable 'unused'",
                 SourceRange(1, 5, 6));
    d.addNote("variable declared here but never used");
    d.addHelp("prefix the name with `_` to suppress this warning");
    engine.report(d);

    std::string out = stripAnsi(engine.format(engine.diagnostics().front()));

    CHECK(out.find("warning[W001]: unused variable 'unused'") != std::string::npos);
    CHECK(out.find("~~~~~~") != std::string::npos);
    CHECK(out.find("= note: variable declared here but never used") != std::string::npos);
    CHECK(out.find("= help: prefix the name with") != std::string::npos);
}

// ---- Integration: Lexer reports through DiagnosticEngine -------------------

TEST_CASE("Lexer integrates with DiagnosticEngine", "[diagnostics][lexer]") {
    SECTION("Unexpected character is reported to engine before throw") {
        const std::string src = "var x = @;";
        DiagnosticEngine engine(src, false);
        Lexer lexer(src, &engine);

        REQUIRE_THROWS_AS(lexer.scanTokens(), LexerError);
        REQUIRE(engine.hasErrors());
        REQUIRE(engine.errorCount() == 1);
        REQUIRE(engine.diagnostics().front().code == "E001");
    }

    SECTION("Unterminated string is reported to engine before throw") {
        const std::string src = "var s = \"hello";
        DiagnosticEngine engine(src, false);
        Lexer lexer(src, &engine);

        REQUIRE_THROWS_AS(lexer.scanTokens(), LexerError);
        REQUIRE(engine.hasErrors());
        REQUIRE(engine.diagnostics().front().code == "E002");
    }

    SECTION("Unterminated block comment is reported to engine before throw") {
        const std::string src = "/* no end";
        DiagnosticEngine engine(src, false);
        Lexer lexer(src, &engine);

        REQUIRE_THROWS_AS(lexer.scanTokens(), LexerError);
        REQUIRE(engine.hasErrors());
        REQUIRE(engine.diagnostics().front().code == "E003");
    }

    SECTION("Lexer without engine still throws normally") {
        Lexer lexer("var x = @;");
        REQUIRE_THROWS_AS(lexer.scanTokens(), LexerError);
    }
}

// ---- Integration: Parser reports through DiagnosticEngine ------------------

TEST_CASE("Parser integrates with DiagnosticEngine", "[diagnostics][parser]") {
    SECTION("Parse error is reported to engine") {
        const std::string src = "var x = ;";
        DiagnosticEngine engine(src, false);
        Lexer lexer(src);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), src, &engine);

        // parse() catches exceptions internally via synchronize(); check engine
        parser.parse();
        REQUIRE(engine.hasErrors());
        REQUIRE(engine.diagnostics().front().code == "E100");
    }

    SECTION("Parser without engine still produces parse errors (via synchronize)") {
        const std::string src = "var x = ;";
        Lexer lexer(src);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens), src);
        // Should not throw (parser synchronizes internally)
        REQUIRE_NOTHROW(parser.parse());
    }
}

// ---- Edge cases -------------------------------------------------------------

TEST_CASE("DiagnosticEngine: edge cases", "[diagnostics]") {
    SECTION("Zero-length source produces no lines") {
        DiagnosticEngine engine("", false);
        Diagnostic d(DiagnosticLevel::Error, "E001", "error on empty source", SourceRange(1, 1, 1));
        engine.report(d);
        // Should not crash; source line will be empty
        std::string out = engine.format(engine.diagnostics().front());
        REQUIRE(!out.empty());
    }

    SECTION("Out-of-range line number does not crash") {
        DiagnosticEngine engine("var x = 1;", false);
        engine.error("E001", "error on line 99", SourceRange(99, 1, 1));
        // format should not crash even if line 99 does not exist
        REQUIRE_NOTHROW(engine.format(engine.diagnostics().front()));
    }

    SECTION("Diagnostic without code omits brackets in output") {
        DiagnosticEngine engine("var x = 1;", false);
        engine.note("consider using a constant", SourceRange(1, 5, 1));
        std::string out = stripAnsi(engine.format(engine.diagnostics().front()));
        // No code -> no brackets
        REQUIRE(out.find("note: consider") != std::string::npos);
        REQUIRE(out.find("note[]") == std::string::npos);
    }
}

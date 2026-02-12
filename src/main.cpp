#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <filesystem>
#include <vector>

#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "common/error_reporter.hpp"
#include "common/cli.hpp"
#include "common/semantic_analyzer.hpp"

using namespace izi;
namespace fs = std::filesystem;


void runCode(const std::string& src, bool useVM, bool debug, const std::string& filename = "<stdin>") {
    try {
        if (debug) {
            std::cout << "[DEBUG] Lexing and parsing...\n";
        }
        
        Lexer lex(src);
        auto tokens = lex.scanTokens();
        Parser parser(std::move(tokens), src);
        auto program = parser.parse();

        if (debug) {
            std::cout << "[DEBUG] Execution mode: " << (useVM ? "VM" : "Interpreter") << "\n";
        }

        if (!useVM) {
            Interpreter interp(src);
            interp.interpret(program);
        } else {
            std::unordered_set<std::string> importedModules;
            BytecodeCompiler compiler;
            compiler.setImportedModules(&importedModules);
            Chunk chunk = compiler.compile(program);
            VM vm;
            registerVmNatives(vm);
            Value result = vm.run(chunk);
        }
    } catch (const LexerError& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.line, e.column, e.what(), "Lexer Error") << '\n';
        throw;
    } catch (const ParserError& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.token, e.what(), "Parse Error") << '\n';
        throw;
    } catch (const RuntimeError& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.token, e.what()) << '\n';
        throw;
    } catch (const ThrowSignal& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.token, "Uncaught exception") << '\n';
        std::cerr << "Exception value: ";
        printValue(e.exception);
        std::cerr << '\n';
        throw;
    }
}

void runRepl(bool useVM, bool debug) {
    std::cout << IZILANG_VERSION << " REPL\n";
    std::cout << "Type 'exit()' or press Ctrl+D to quit\n";
    std::cout << "Type ':help' for REPL commands\n\n";

    // Set of valid exit commands
    const std::unordered_set<std::string> exitCommands = {"exit()", "exit", "quit()", "quit"};
    
    // Create a persistent interpreter for the REPL session
    Interpreter* interp = nullptr;
    VM* vm = nullptr;
    std::unordered_set<std::string> importedModules;
    
    if (!useVM) {
        interp = new Interpreter("");
    } else {
        vm = new VM();
        registerVmNatives(*vm);
    }

    std::string line;
    std::string multilineBuffer;
    bool inMultiline = false;
    
    while (true) {
        // Show appropriate prompt
        if (inMultiline) {
            std::cout << "... ";
        } else {
            std::cout << "> ";
        }
        std::cout.flush();
        
        if (!std::getline(std::cin, line)) {
            // EOF or error
            std::cout << "\n";
            break;
        }

        // Check for REPL special commands (only when not in multiline)
        if (!inMultiline && line.length() > 0 && line[0] == ':') {
            if (line == ":help") {
                std::cout << "\nREPL Commands:\n";
                std::cout << "  :help      Show this help message\n";
                std::cout << "  :exit      Exit the REPL\n";
                std::cout << "  :reset     Reset the REPL environment\n";
                std::cout << "  :debug     Toggle debug mode\n";
                std::cout << "\nTo exit, you can also use: exit(), quit(), or Ctrl+D\n\n";
                continue;
            } else if (line == ":exit") {
                break;
            } else if (line == ":reset") {
                // Reset the interpreter/VM
                if (!useVM) {
                    delete interp;
                    interp = new Interpreter("");
                } else {
                    delete vm;
                    vm = new VM();
                    registerVmNatives(*vm);
                }
                importedModules.clear();
                std::cout << "REPL environment reset.\n";
                continue;
            } else if (line == ":debug") {
                debug = !debug;
                std::cout << "Debug mode " << (debug ? "enabled" : "disabled") << ".\n";
                continue;
            } else {
                std::cout << "Unknown command: " << line << "\n";
                std::cout << "Type ':help' for available commands.\n";
                continue;
            }
        }

        // Skip empty lines when not in multiline
        if (!inMultiline && line.empty()) {
            continue;
        }

        // Check for exit command
        if (!inMultiline && exitCommands.count(line) > 0) {
            break;
        }
        
        // Handle multi-line input
        // Simple heuristic: if line ends with { or (, expect more input
        // Note: This is a simple approach and may incorrectly trigger for
        // strings/comments containing these characters. A more robust solution
        // would parse the line to check if the brace/paren is in code context.
        bool lineEndsWithBrace = !line.empty() && (line.back() == '{' || line.back() == '(');
        
        if (lineEndsWithBrace || inMultiline) {
            if (!inMultiline) {
                multilineBuffer = line + "\n";
                inMultiline = true;
            } else {
                multilineBuffer += line + "\n";
                // Check if we should exit multiline mode
                // Simple heuristic: if line ends with } or ), try to execute
                if (!line.empty() && (line.back() == '}' || line.back() == ')')) {
                    inMultiline = false;
                    line = multilineBuffer;
                    multilineBuffer.clear();
                } else {
                    continue;
                }
            }
        }
        
        // Don't continue if still in multiline mode
        if (inMultiline) {
            continue;
        }

        try {
            // Note: Currently runCode() creates a new interpreter/VM each time,
            // which limits state persistence in the REPL. The persistent
            // interpreter/VM created above could be used for true state
            // preservation, but would require refactoring runCode() to accept
            // an existing interpreter/VM instance.
            runCode(line, useVM, debug, "<repl>");
        } catch (const std::exception& e) {
            // Most errors are already printed by runCode
            // This catch is for unexpected std::exception types
            if (debug) {
                std::cerr << "[DEBUG] Caught exception: " << e.what() << "\n";
            }
        } catch (...) {
            // Catch any other exceptions and continue REPL
            if (debug) {
                std::cerr << "[DEBUG] Caught unknown exception\n";
            }
        }
    }
    
    // Cleanup
    if (interp) delete interp;
    if (vm) delete vm;
}

int runTests(const CliOptions& options) {
    bool useVM = (options.engine == CliOptions::Engine::VM);
    
    // Determine test directories
    std::vector<std::string> testDirs = {"examples", "tests"};
    std::vector<fs::path> testFiles;
    
    // Collect all .iz files from test directories
    for (const auto& dir : testDirs) {
        if (fs::exists(dir) && fs::is_directory(dir)) {
            for (const auto& entry : fs::directory_iterator(dir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".iz") {
                    // Apply pattern filter if provided
                    bool match = true;
                    if (!options.args.empty()) {
                        match = false;
                        for (const auto& pattern : options.args) {
                            if (entry.path().filename().string().find(pattern) != std::string::npos) {
                                match = true;
                                break;
                            }
                        }
                    }
                    if (match) {
                        testFiles.push_back(entry.path());
                    }
                }
            }
        }
    }
    
    if (testFiles.empty()) {
        std::cout << "No test files found.\n";
        if (!options.args.empty()) {
            std::cout << "Searched for pattern(s): ";
            for (size_t i = 0; i < options.args.size(); i++) {
                if (i > 0) std::cout << ", ";
                std::cout << "'" << options.args[i] << "'";
            }
            std::cout << "\n";
        }
        return 0;
    }
    
    // Run tests
    int passed = 0;
    int failed = 0;
    std::vector<std::string> failedTests;
    
    std::cout << "Running " << testFiles.size() << " test file(s)...\n\n";
    
    for (const auto& testFile : testFiles) {
        std::cout << "Testing: " << testFile.string() << " ... ";
        std::cout.flush();
        
        try {
            std::ifstream f(testFile);
            if (!f.is_open()) {
                std::cout << "FAILED (cannot open file)\n";
                failed++;
                failedTests.push_back(testFile.string());
                continue;
            }
            
            std::stringstream buffer;
            buffer << f.rdbuf();
            std::string src = buffer.str();
            
            // Redirect stdout temporarily to capture output
            std::stringstream capturedOutput;
            std::streambuf* oldCout = std::cout.rdbuf(capturedOutput.rdbuf());
            
            try {
                runCode(src, useVM, options.debug, testFile.string());
                
                // Restore stdout
                std::cout.rdbuf(oldCout);
                
                std::cout << "PASSED\n";
                if (options.debug) {
                    std::cout << "  Output:\n" << capturedOutput.str();
                }
                passed++;
            } catch (...) {
                // Restore stdout
                std::cout.rdbuf(oldCout);
                
                std::cout << "FAILED\n";
                failed++;
                failedTests.push_back(testFile.string());
            }
        } catch (const std::exception& e) {
            std::cout << "FAILED (" << e.what() << ")\n";
            failed++;
            failedTests.push_back(testFile.string());
        }
    }
    
    // Print summary
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "Test Results\n";
    std::cout << "========================================\n";
    std::cout << "Passed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    std::cout << "Total:  " << (passed + failed) << "\n";
    
    if (failed > 0) {
        std::cout << "\nFailed tests:\n";
        for (const auto& test : failedTests) {
            std::cout << "  - " << test << "\n";
        }
    }
    
    return (failed > 0) ? 1 : 0;
}

int main(int argc, char** argv) {
    CliOptions options = CliOptions::parse(argc, argv);

    // Handle version and help
    if (options.command == CliOptions::Command::Version) {
        CliOptions::printVersion();
        return 0;
    }

    if (options.command == CliOptions::Command::Help) {
        // Help was either already printed or needs to be printed now
        // The parse() function handles specific command help, but general help
        // needs to be printed here
        return 0;
    }

    bool useVM = (options.engine == CliOptions::Engine::VM);

    // Handle REPL mode
    if (options.command == CliOptions::Command::Repl) {
        runRepl(useVM, options.debug);
        return 0;
    }

    // Handle test command
    if (options.command == CliOptions::Command::Test) {
        return runTests(options);
    }

    // Handle fmt command
    if (options.command == CliOptions::Command::Fmt) {
        std::cerr << "Error: 'fmt' command not yet implemented\n";
        std::cerr << "This feature is planned for v0.2\n";
        return 1;
    }

    // Get source code from file
    std::ifstream f(options.input);
    if (!f.is_open()) {
        std::cerr << "Cannot open file: " << options.input << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string src = buffer.str();

    // Handle different commands
    if (options.command == CliOptions::Command::Run) {
        // Execute the code
        try {
            runCode(src, useVM, options.debug, options.input);
        } catch (...) {
            return 1;
        }
    } else if (options.command == CliOptions::Command::Build) {
        // Compile/check only, no execution
        try {
            if (options.debug) {
                std::cout << "[DEBUG] Building (compile-only mode)...\n";
            }
            
            Lexer lex(src);
            auto tokens = lex.scanTokens();
            
            if (options.debug) {
                std::cout << "[DEBUG] Lexing complete\n";
            }
            
            Parser parser(std::move(tokens), src);
            auto program = parser.parse();
            
            if (options.debug) {
                std::cout << "[DEBUG] Parsing complete\n";
            }
            
            // Compile to bytecode to check for compilation errors
            std::unordered_set<std::string> importedModules;
            BytecodeCompiler compiler;
            compiler.setImportedModules(&importedModules);
            Chunk chunk = compiler.compile(program);
            
            if (options.debug) {
                std::cout << "[DEBUG] Compilation complete\n";
            }
            
            std::cout << "Build successful: " << options.input << "\n";
        } catch (const LexerError& e) {
            ErrorReporter reporter(src);
            std::cerr << "In file '" << options.input << "':\n";
            std::cerr << reporter.formatError(e.line, e.column, e.what(), "Lexer Error") << '\n';
            return 1;
        } catch (const ParserError& e) {
            ErrorReporter reporter(src);
            std::cerr << "In file '" << options.input << "':\n";
            std::cerr << reporter.formatError(e.token, e.what(), "Parse Error") << '\n';
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Build failed: " << e.what() << '\n';
            return 1;
        }
    } else if (options.command == CliOptions::Command::Check) {
        // Parse and analyze only, no compilation
        try {
            if (options.debug) {
                std::cout << "[DEBUG] Checking (parse-only mode)...\n";
            }
            
            Lexer lex(src);
            auto tokens = lex.scanTokens();
            
            if (options.debug) {
                std::cout << "[DEBUG] Lexing complete, " << tokens.size() << " tokens\n";
            }
            
            Parser parser(std::move(tokens), src);
            auto program = parser.parse();
            
            if (options.debug) {
                std::cout << "[DEBUG] Parsing complete, " << program.size() << " statements\n";
            }
            
            // Run semantic analysis
            SemanticAnalyzer analyzer;
            analyzer.analyze(program);
            
            if (options.debug) {
                std::cout << "[DEBUG] Semantic analysis complete\n";
            }
            
            // Report diagnostics
            const auto& diagnostics = analyzer.getDiagnostics();
            ErrorReporter reporter(src);
            
            bool hasErrors = false;
            for (const auto& diag : diagnostics) {
                std::string filePrefix = "In file '" + options.input + "':\n";
                
                if (diag.severity == SemanticDiagnostic::Severity::Error) {
                    hasErrors = true;
                    std::cerr << filePrefix;
                    std::cerr << reporter.formatError(diag.line, diag.column, diag.message, "Semantic Error") << '\n';
                } else if (diag.severity == SemanticDiagnostic::Severity::Warning) {
                    std::cerr << filePrefix;
                    std::cerr << reporter.formatError(diag.line, diag.column, diag.message, "Warning") << '\n';
                } else {
                    // Info messages
                    std::cout << filePrefix;
                    std::cout << reporter.formatError(diag.line, diag.column, diag.message, "Info") << '\n';
                }
            }
            
            if (hasErrors) {
                return 1;
            }
            
            std::cout << "Check successful: " << options.input << "\n";
        } catch (const LexerError& e) {
            ErrorReporter reporter(src);
            std::cerr << "In file '" << options.input << "':\n";
            std::cerr << reporter.formatError(e.line, e.column, e.what(), "Lexer Error") << '\n';
            return 1;
        } catch (const ParserError& e) {
            ErrorReporter reporter(src);
            std::cerr << "In file '" << options.input << "':\n";
            std::cerr << reporter.formatError(e.token, e.what(), "Parse Error") << '\n';
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Check failed: " << e.what() << '\n';
            return 1;
        }
    }

    return 0;
};
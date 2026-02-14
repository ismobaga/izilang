#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <filesystem>
#include <vector>
#include <chrono>

#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "compile/optimizer.hpp"
#include "compile/native_compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "bytecode/chunk_serializer.hpp"
#include "common/error_reporter.hpp"
#include "common/cli.hpp"
#include "common/semantic_analyzer.hpp"

using namespace izi;
namespace fs = std::filesystem;


void runCode(const std::string& src, bool useVM, bool debug, bool optimize, const std::string& filename = "<stdin>", const std::vector<std::string>& args = {}) {
    try {
        if (debug) {
            std::cout << "[DEBUG] Lexing and parsing...\n";
        }
        
        Lexer lex(src);
        auto tokens = lex.scanTokens();
        Parser parser(std::move(tokens), src);
        auto program = parser.parse();

        // Apply optimizations if enabled
        if (optimize) {
            if (debug) {
                std::cout << "[DEBUG] Applying optimizations...\n";
            }
            Optimizer optimizer;
            program = optimizer.optimize(std::move(program));
        }

        if (debug) {
            std::cout << "[DEBUG] Execution mode: " << (useVM ? "VM" : "Interpreter") << "\n";
        }

        if (!useVM) {
            Interpreter interp(src);
            interp.setCurrentFile(filename);  // Set current file for relative imports
            interp.setCommandLineArgs(args);
            interp.interpret(program);
        } else {
            std::unordered_set<std::string> importedModules;
            BytecodeCompiler compiler;
            compiler.setCurrentFile(filename);  // Set current file for relative imports
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
    } catch (const std::runtime_error& e) {
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << "Error: " << e.what() << '\n';
        throw;
    }
}

void runReplLine(const std::string& src, Interpreter* interp, VM* vm, bool useVM, bool debug, bool optimize, const std::string& filename = "<repl>") {
    try {
        if (debug) {
            std::cout << "[DEBUG] Lexing and parsing...\n";
        }
        
        Lexer lex(src);
        auto tokens = lex.scanTokens();
        Parser parser(std::move(tokens), src);
        auto program = parser.parse();

        // Apply optimizations if enabled
        if (optimize) {
            if (debug) {
                std::cout << "[DEBUG] Applying optimizations...\n";
            }
            Optimizer optimizer;
            program = optimizer.optimize(std::move(program));
        }

        if (debug) {
            std::cout << "[DEBUG] Execution mode: " << (useVM ? "VM" : "Interpreter") << "\n";
        }

        if (!useVM && interp) {
            interp->interpret(program);
        } else if (useVM && vm) {
            std::unordered_set<std::string> importedModules;
            BytecodeCompiler compiler;
            compiler.setCurrentFile(filename);
            compiler.setImportedModules(&importedModules);
            Chunk chunk = compiler.compile(program);
            Value result = vm->run(chunk);
        }
    } catch (const LexerError& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.line, e.column, e.what(), "Lexer Error") << '\n';
        // Don't rethrow in REPL, just continue
    } catch (const ParserError& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.token, e.what(), "Parse Error") << '\n';
        // Don't rethrow in REPL, just continue
    } catch (const RuntimeError& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.token, e.what()) << '\n';
        // Don't rethrow in REPL, just continue
    } catch (const ThrowSignal& e) {
        ErrorReporter reporter(src);
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << reporter.formatError(e.token, "Uncaught exception") << '\n';
        std::cerr << "Exception value: ";
        printValue(e.exception);
        std::cerr << '\n';
        // Don't rethrow in REPL, just continue
    } catch (const std::runtime_error& e) {
        std::cerr << "In file '" << filename << "':\n";
        std::cerr << "Error: " << e.what() << '\n';
        // Don't rethrow in REPL, just continue
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
                std::cout << "  :vars      Show all defined variables\n";
                std::cout << "  :tasks     Show async tasks (not yet implemented)\n";
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
            } else if (line == ":vars") {
                std::cout << "\nDefined Variables:\n";
                if (!useVM) {
                    if (interp) {
                        auto globals = interp->getGlobals();
                        if (globals) {
                            const auto& vars = globals->getAll();
                            if (vars.empty()) {
                                std::cout << "  (no variables defined)\n";
                            } else {
                                for (const auto& [name, value] : vars) {
                                    std::cout << "  " << name << " = ";
                                    printValue(value);
                                    std::cout << "\n";
                                }
                            }
                        }
                    }
                } else {
                    if (vm) {
                        const auto& vars = vm->getGlobals();
                        if (vars.empty()) {
                            std::cout << "  (no variables defined)\n";
                        } else {
                            for (const auto& [name, value] : vars) {
                                std::cout << "  " << name << " = ";
                                printValue(value);
                                std::cout << "\n";
                            }
                        }
                    }
                }
                std::cout << "\n";
                continue;
            } else if (line == ":tasks") {
                std::cout << "\nAsync Tasks:\n";
                std::cout << "  (async/task support not yet implemented)\n\n";
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
            // Use the persistent interpreter/VM for the REPL
            runReplLine(line, interp, vm, useVM, debug, true, "<repl>");
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
                runCode(src, useVM, options.debug, options.optimize, testFile.string());
                
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

int runBenchmark(const CliOptions& options) {
    bool useVM = (options.engine == CliOptions::Engine::VM);
    int iterations = 5;  // Default iterations
    
    // Parse iterations from args if provided
    for (size_t i = 0; i < options.args.size(); ++i) {
        if (options.args[i] == "--iterations" && i + 1 < options.args.size()) {
            try {
                iterations = std::stoi(options.args[i + 1]);
                if (iterations <= 0) {
                    std::cerr << "Error: iterations must be positive\n";
                    return 1;
                }
            } catch (...) {
                std::cerr << "Error: invalid iterations value\n";
                return 1;
            }
        }
    }
    
    // Read benchmark file
    std::ifstream f(options.input);
    if (!f.is_open()) {
        std::cerr << "Cannot open file: " << options.input << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string src = buffer.str();
    
    std::cout << "╔════════════════════════════════════════╗\n";
    std::cout << "║   IziLang Performance Benchmark       ║\n";
    std::cout << "╚════════════════════════════════════════╝\n\n";
    std::cout << "File: " << options.input << "\n";
    std::cout << "Engine: " << (useVM ? "VM" : "Interpreter") << "\n";
    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Optimizations: " << (options.optimize ? "enabled" : "disabled") << "\n\n";
    
    std::cout << "Running benchmark";
    std::cout.flush();
    
    // Measure execution time
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < iterations; ++i) {
        try {
            // Redirect stdout to /dev/null during benchmark to avoid printing overhead
            std::stringstream nullStream;
            std::streambuf* oldCout = std::cout.rdbuf(nullStream.rdbuf());
            
            runCode(src, useVM, false, options.optimize, options.input);
            
            std::cout.rdbuf(oldCout);
            std::cout << ".";
            std::cout.flush();
        } catch (...) {
            std::cout.rdbuf(std::cout.rdbuf());
            std::cerr << "\n\nError: Benchmark failed during execution\n";
            return 1;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double totalMs = duration.count();
    double avgMs = totalMs / iterations;
    
    std::cout << " done!\n\n";
    std::cout << "Results:\n";
    std::cout << "  Total time:   " << totalMs << " ms\n";
    std::cout << "  Average time: " << avgMs << " ms\n";
    std::cout << "  Throughput:   " << (1000.0 / avgMs) << " runs/sec\n\n";
    
    return 0;
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
    
    // Handle bench command
    if (options.command == CliOptions::Command::Bench) {
        return runBenchmark(options);
    }

    // Handle fmt command
    if (options.command == CliOptions::Command::Fmt) {
        std::cerr << "Error: 'fmt' command not yet implemented\n";
        std::cerr << "This feature is planned for v0.2\n";
        return 1;
    }

    // Handle compile command
    if (options.command == CliOptions::Command::Compile) {
        NativeCompiler::CompileOptions compileOpts;
        compileOpts.inputFile = options.input;
        compileOpts.debug = options.debug;
        compileOpts.verbose = options.debug;
        
        // Determine output filename
        if (!options.output.empty()) {
            compileOpts.outputFile = options.output;
        } else {
            // Default: strip .iz extension and use as executable name
            fs::path inputPath(options.input);
            compileOpts.outputFile = inputPath.stem().string();
        }
        
        // Perform compilation
        bool success = NativeCompiler::compile(compileOpts);
        return success ? 0 : 1;
    }

    // Handle chunk command (compile to .izb bytecode)
    if (options.command == CliOptions::Command::Chunk) {
        try {
            if (options.debug) {
                std::cout << "[DEBUG] Compiling to bytecode chunk...\n";
            }
            
            // Read source file
            std::ifstream f(options.input);
            if (!f.is_open()) {
                std::cerr << "Cannot open file: " << options.input << "\n";
                return 1;
            }
            std::stringstream buffer;
            buffer << f.rdbuf();
            std::string src = buffer.str();
            
            // Lex and parse
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
            
            // Apply optimizations if enabled
            if (options.optimize) {
                if (options.debug) {
                    std::cout << "[DEBUG] Applying optimizations...\n";
                }
                Optimizer optimizer;
                program = optimizer.optimize(std::move(program));
            }
            
            // Compile to bytecode
            std::unordered_set<std::string> importedModules;
            BytecodeCompiler compiler;
            
            // Convert input file to absolute path for proper relative import resolution
            std::string absolutePath;
            try {
                absolutePath = fs::absolute(options.input).string();
            } catch (const fs::filesystem_error&) {
                absolutePath = options.input;
            }
            compiler.setCurrentFile(absolutePath);
            compiler.setImportedModules(&importedModules);
            
            Chunk chunk = compiler.compile(program);
            
            if (options.debug) {
                std::cout << "[DEBUG] Bytecode compilation complete\n";
                std::cout << "[DEBUG] Code size: " << chunk.code.size() << " bytes\n";
                std::cout << "[DEBUG] Constants: " << chunk.constants.size() << "\n";
                std::cout << "[DEBUG] Names: " << chunk.names.size() << "\n";
            }
            
            // Determine output filename
            std::string outputFile;
            if (!options.output.empty()) {
                outputFile = options.output;
            } else {
                // Default: replace .iz with .izb
                fs::path inputPath(options.input);
                outputFile = inputPath.stem().string() + ".izb";
            }
            
            // Serialize to file
            if (!ChunkSerializer::serializeToFile(chunk, outputFile)) {
                std::cerr << "Failed to write bytecode file: " << outputFile << "\n";
                return 1;
            }
            
            std::cout << "Successfully compiled to: " << outputFile << "\n";
            
        } catch (const LexerError& e) {
            std::cerr << "In file '" << options.input << "':\n";
            std::cerr << "Lexer Error: " << e.what() << '\n';
            return 1;
        } catch (const ParserError& e) {
            std::cerr << "In file '" << options.input << "':\n";
            std::cerr << "Parse Error: " << e.what() << '\n';
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Compilation failed: " << e.what() << '\n';
            return 1;
        }
        
        return 0;
    }

    // Get source code from file (skip for .izb files)
    std::ifstream f(options.input);
    if (!f.is_open()) {
        std::cerr << "Cannot open file: " << options.input << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string src = buffer.str();
    
    // Convert input file to absolute path for proper relative import resolution
    std::string absolutePath;
    try {
        absolutePath = fs::absolute(options.input).string();
    } catch (const fs::filesystem_error&) {
        // If canonicalization fails, use the original path
        absolutePath = options.input;
    }

    // Handle different commands
    if (options.command == CliOptions::Command::Run) {
        // Check if input is a .izb bytecode file
        fs::path inputPath(options.input);
        if (inputPath.extension() == ".izb") {
            // Load and execute bytecode chunk directly
            if (!useVM) {
                std::cerr << "Error: .izb files can only be executed with the VM.\n";
                std::cerr << "Use: izi run --vm " << options.input << "\n";
                return 1;
            }
            
            try {
                if (options.debug) {
                    std::cout << "[DEBUG] Loading bytecode chunk from " << options.input << "\n";
                }
                
                Chunk chunk = ChunkSerializer::deserializeFromFile(options.input);
                
                if (options.debug) {
                    std::cout << "[DEBUG] Chunk loaded successfully\n";
                    std::cout << "[DEBUG] Code size: " << chunk.code.size() << " bytes\n";
                    std::cout << "[DEBUG] Constants: " << chunk.constants.size() << "\n";
                    std::cout << "[DEBUG] Names: " << chunk.names.size() << "\n";
                }
                
                VM vm;
                registerVmNatives(vm);
                Value result = vm.run(chunk);
                
                if (options.debug) {
                    std::cout << "[DEBUG] Execution complete\n";
                }
            } catch (const std::exception& e) {
                std::cerr << "Error executing bytecode: " << e.what() << '\n';
                return 1;
            }
            return 0;
        }
        
        // Execute the code (source file)
        try {
            // Build command line arguments: script name followed by additional args
            std::vector<std::string> cmdArgs;
            cmdArgs.push_back(options.input);  // Script name as first argument
            cmdArgs.insert(cmdArgs.end(), options.args.begin(), options.args.end());
            
            runCode(src, useVM, options.debug, options.optimize, absolutePath, cmdArgs);
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
            
            // Apply optimizations if enabled
            if (options.optimize) {
                if (options.debug) {
                    std::cout << "[DEBUG] Applying optimizations...\n";
                }
                Optimizer optimizer;
                program = optimizer.optimize(std::move(program));
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
            std::string filePrefix = "In file '" + options.input + "':\n";
            
            bool hasErrors = false;
            for (const auto& diag : diagnostics) {
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
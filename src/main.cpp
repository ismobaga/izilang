#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "common/error_reporter.hpp"
#include "common/cli.hpp"

using namespace izi;


void runCode(const std::string& src, bool useVM, bool debug) {
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
        std::cerr << reporter.formatError(e.line, e.column, e.what(), "Lexer Error") << '\n';
        throw;
    } catch (const ParserError& e) {
        ErrorReporter reporter(src);
        std::cerr << reporter.formatError(e.token, e.what(), "Parse Error") << '\n';
        throw;
    } catch (const RuntimeError& e) {
        ErrorReporter reporter(src);
        std::cerr << reporter.formatError(e.token, e.what()) << '\n';
        throw;
    } catch (const ThrowSignal& e) {
        ErrorReporter reporter(src);
        std::cerr << reporter.formatError(e.token, "Uncaught exception") << '\n';
        std::cerr << "Exception value: ";
        printValue(e.exception);
        std::cerr << '\n';
        throw;
    }
}

void runRepl(bool useVM, bool debug) {
    std::cout << IZILANG_VERSION << " REPL\n";
    std::cout << "Type 'exit()' or press Ctrl+D to quit\n\n";

    // Set of valid exit commands
    const std::unordered_set<std::string> exitCommands = {"exit()", "exit", "quit()", "quit"};

    std::string line;
    while (true) {
        std::cout << "> ";
        std::cout.flush();
        
        if (!std::getline(std::cin, line)) {
            // EOF or error
            std::cout << "\n";
            break;
        }

        // Skip empty lines
        if (line.empty()) {
            continue;
        }

        // Check for exit command
        if (exitCommands.count(line) > 0) {
            break;
        }

        try {
            runCode(line, useVM, debug);
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
}

int main(int argc, char** argv) {
    CliOptions options = CliOptions::parse(argc, argv);

    // Handle version and help
    if (options.mode == CliOptions::ExecutionMode::Version) {
        CliOptions::printVersion();
        return 0;
    }

    if (options.mode == CliOptions::ExecutionMode::Help) {
        CliOptions::printHelp();
        return 0;
    }

    bool useVM = (options.engine == CliOptions::Engine::VM);

    // Handle REPL mode
    if (options.mode == CliOptions::ExecutionMode::Repl) {
        runRepl(useVM, options.debug);
        return 0;
    }

    // Get source code
    std::string src;

    if (options.mode == CliOptions::ExecutionMode::Inline) {
        // Inline code
        src = options.input;
    } else {
        // File mode
        std::ifstream f(options.input);
        if (!f.is_open()) {
            std::cerr << "Cannot open file: " << options.input << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << f.rdbuf();
        src = buffer.str();
    }

    // Run the code
    try {
        runCode(src, useVM, options.debug);
    } catch (...) {
        return 1;
    }

    return 0;
};
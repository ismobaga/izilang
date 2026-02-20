#include "common/cli.hpp"
#include <iostream>
#include <cstring>

namespace izi {

void CliOptions::printVersion() {
    std::cout << IZILANG_VERSION << "\n";
}

void CliOptions::printHelp() {
    std::cout << "IziLang - A modern, expressive programming language\n\n";
    std::cout << "Usage: izi <command> [options] [arguments]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  run <file>          Execute a source file (default command)\n";
    std::cout << "  build <file>        Compile/check without executing\n";
    std::cout << "  check <file>        Parse and analyze without executing\n";
    std::cout << "  compile <file>      Compile to native executable\n";
    std::cout << "  chunk <file>        Compile to bytecode chunk (.izb)\n";
    std::cout << "  test [pattern]      Run test files (searches for *.iz in tests/)\n";
    std::cout << "  repl                Start interactive REPL\n";
    std::cout << "  bench <file>        Run performance benchmark\n";
    std::cout << "  fmt <file>          Format source code (coming soon)\n";
    std::cout << "  version             Show version information\n";
    std::cout << "  help [command]      Show help for a specific command\n";
    std::cout << "\n";
    std::cout << "Global Options:\n";
    std::cout << "  --vm                Use bytecode VM (default: tree-walker)\n";
    std::cout << "  --interp            Use tree-walker interpreter (default)\n";
    std::cout << "  --debug             Enable debug/verbose output\n";
    std::cout << "  --optimize, -O      Enable optimizations (default: on)\n";
    std::cout << "  --no-optimize, -O0  Disable optimizations\n";
    std::cout << "  --memory-stats      Show memory usage statistics (debug)\n";
    std::cout << "  --help, -h          Show this help message\n";
    std::cout << "  --version, -v       Show version information\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  izi                 Start interactive REPL\n";
    std::cout << "  izi run script.iz   Run a script file\n";
    std::cout << "  izi script.iz       Run a script file (shorthand)\n";
    std::cout << "  izi build app.iz    Check syntax without running\n";
    std::cout << "  izi compile app.iz  Compile to native executable\n";
    std::cout << "  izi chunk app.iz -o app.izb  Compile to bytecode chunk\n";
    std::cout << "  izi test            Run all tests\n";
    std::cout << "  izi repl            Start REPL explicitly\n";
    std::cout << "\n";
    std::cout << "For more information about a specific command, use:\n";
    std::cout << "  izi help <command>\n";
}

void CliOptions::printCommandHelp(Command cmd) {
    switch (cmd) {
        case Command::Run:
            std::cout << "izi run - Execute a source file\n\n";
            std::cout << "Usage: izi run [options] <file>\n\n";
            std::cout << "Options:\n";
            std::cout << "  --vm       Use bytecode VM\n";
            std::cout << "  --interp   Use tree-walker interpreter (default)\n";
            std::cout << "  --debug    Enable debug output\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi run script.iz\n";
            std::cout << "  izi run --vm script.iz\n";
            break;

        case Command::Build:
            std::cout << "izi build - Compile/check without executing\n\n";
            std::cout << "Usage: izi build [options] <file>\n\n";
            std::cout << "Description:\n";
            std::cout << "  Parses and compiles the source file, reporting any syntax\n";
            std::cout << "  or compilation errors without actually running the code.\n";
            std::cout << "  Useful for checking code before execution.\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  --debug    Show compilation details\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi build app.iz\n";
            break;

        case Command::Check:
            std::cout << "izi check - Parse and analyze without executing\n\n";
            std::cout << "Usage: izi check [options] <file>\n\n";
            std::cout << "Description:\n";
            std::cout << "  Performs syntax checking and basic static analysis\n";
            std::cout << "  without compilation or execution. Fastest way to\n";
            std::cout << "  validate syntax.\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  --debug    Show parse tree details\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi check script.iz\n";
            break;

        case Command::Compile:
            std::cout << "izi compile - Compile to native executable\n\n";
            std::cout << "Usage: izi compile [options] <file> [-o <output>]\n\n";
            std::cout << "Description:\n";
            std::cout << "  Compiles an IziLang source file into a standalone\n";
            std::cout << "  native executable with no runtime dependencies.\n";
            std::cout << "  Uses static linking for portability.\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  -o <output>  Specify output executable name\n";
            std::cout << "  --debug      Include debug symbols\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi compile app.iz\n";
            std::cout << "  izi compile app.iz -o myapp\n";
            std::cout << "  izi compile --debug app.iz\n";
            break;

        case Command::Chunk:
            std::cout << "izi chunk - Compile to bytecode chunk (.izb)\n\n";
            std::cout << "Usage: izi chunk [options] <file> [-o <output>]\n\n";
            std::cout << "Description:\n";
            std::cout << "  Compiles an IziLang source file into a bytecode chunk\n";
            std::cout << "  file (.izb) that can be executed efficiently by the VM.\n";
            std::cout << "  Bytecode chunks skip parsing and are faster to load.\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  -o <output>  Specify output .izb file name\n";
            std::cout << "  --debug      Show compilation details\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi chunk app.iz\n";
            std::cout << "  izi chunk app.iz -o app.izb\n";
            std::cout << "  izi chunk --debug script.iz\n";
            std::cout << "\n";
            std::cout << "To run a .izb file:\n";
            std::cout << "  izi run --vm app.izb\n";
            break;

        case Command::Test:
            std::cout << "izi test - Run test files\n\n";
            std::cout << "Usage: izi test [options] [pattern]\n\n";
            std::cout << "Description:\n";
            std::cout << "  Discovers and executes test files in the tests/ directory.\n";
            std::cout << "  Test files should have .iz extension.\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  --vm       Use bytecode VM for tests\n";
            std::cout << "  --debug    Show detailed test output\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi test              Run all tests\n";
            std::cout << "  izi test lexer        Run tests matching 'lexer'\n";
            break;

        case Command::Repl:
            std::cout << "izi repl - Start interactive REPL\n\n";
            std::cout << "Usage: izi repl [options]\n\n";
            std::cout << "Description:\n";
            std::cout << "  Starts an interactive Read-Eval-Print Loop for\n";
            std::cout << "  experimenting with IziLang code.\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  --vm       Use bytecode VM\n";
            std::cout << "  --debug    Enable debug output\n";
            std::cout << "\n";
            std::cout << "REPL Commands:\n";
            std::cout << "  exit() or quit()  Exit the REPL\n";
            std::cout << "  Ctrl+D            Exit the REPL\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi repl\n";
            std::cout << "  izi repl --vm\n";
            break;

        case Command::Fmt:
            std::cout << "izi fmt - Format source code\n\n";
            std::cout << "Usage: izi fmt [options] <file>\n\n";
            std::cout << "Description:\n";
            std::cout << "  Formats IziLang source code according to standard style.\n";
            std::cout << "  (Coming in v0.2 - not yet implemented)\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  --check    Check if file needs formatting (no changes)\n";
            std::cout << "  --write    Write changes to file (default: print to stdout)\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi fmt script.iz\n";
            std::cout << "  izi fmt --write script.iz\n";
            break;

        case Command::Bench:
            std::cout << "izi bench - Run performance benchmark\n\n";
            std::cout << "Usage: izi bench [options] <file>\n\n";
            std::cout << "Description:\n";
            std::cout << "  Runs a benchmark file and measures execution time.\n";
            std::cout << "  Useful for performance testing and optimization validation.\n";
            std::cout << "\n";
            std::cout << "Options:\n";
            std::cout << "  --vm          Use bytecode VM\n";
            std::cout << "  --interp      Use tree-walker interpreter (default)\n";
            std::cout << "  --iterations N  Number of iterations (default: 5)\n";
            std::cout << "\n";
            std::cout << "Examples:\n";
            std::cout << "  izi bench benchmarks/arithmetic.iz\n";
            std::cout << "  izi bench --vm --iterations 10 benchmarks/loops.iz\n";
            break;

        default:
            printHelp();
            break;
    }
}

CliOptions CliOptions::parse(int argc, char** argv) {
    CliOptions options;

    // No arguments -> REPL mode
    if (argc == 1) {
        options.command = Command::Repl;
        return options;
    }

    int i = 1;

    // Check if first argument is a command or option
    std::string firstArg = argv[i];

    // Handle global options first
    if (firstArg == "--version" || firstArg == "-v") {
        options.command = Command::Version;
        return options;
    }

    if (firstArg == "--help" || firstArg == "-h") {
        printHelp();
        options.command = Command::Help;
        return options;
    }

    // Parse subcommand
    if (firstArg == "run") {
        options.command = Command::Run;
        i++;
    } else if (firstArg == "build") {
        options.command = Command::Build;
        i++;
    } else if (firstArg == "check") {
        options.command = Command::Check;
        i++;
    } else if (firstArg == "compile") {
        options.command = Command::Compile;
        i++;
    } else if (firstArg == "chunk") {
        options.command = Command::Chunk;
        i++;
    } else if (firstArg == "test") {
        options.command = Command::Test;
        i++;
    } else if (firstArg == "repl") {
        options.command = Command::Repl;
        i++;
    } else if (firstArg == "bench") {
        options.command = Command::Bench;
        i++;
    } else if (firstArg == "fmt") {
        options.command = Command::Fmt;
        i++;
    } else if (firstArg == "version") {
        options.command = Command::Version;
        return options;
    } else if (firstArg == "help") {
        // Handle "izi help <command>"
        if (i + 1 < argc) {
            std::string helpCmd = argv[i + 1];
            if (helpCmd == "run") {
                printCommandHelp(Command::Run);
            } else if (helpCmd == "build") {
                printCommandHelp(Command::Build);
            } else if (helpCmd == "check") {
                printCommandHelp(Command::Check);
            } else if (helpCmd == "compile") {
                printCommandHelp(Command::Compile);
            } else if (helpCmd == "chunk") {
                printCommandHelp(Command::Chunk);
            } else if (helpCmd == "test") {
                printCommandHelp(Command::Test);
            } else if (helpCmd == "repl") {
                printCommandHelp(Command::Repl);
            } else if (helpCmd == "fmt") {
                printCommandHelp(Command::Fmt);
            } else {
                std::cerr << "Unknown command: " << helpCmd << "\n";
                printHelp();
            }
        } else {
            printHelp();
        }
        options.command = Command::Help;
        return options;
    } else if (firstArg[0] != '-') {
        // No explicit command, but a filename - default to 'run' for backward compatibility
        options.command = Command::Run;
        // Don't increment i, we'll process the filename below
    } else {
        // Unknown option
        std::cerr << "Error: Unknown option or command: " << firstArg << "\n";
        std::cerr << "Use 'izi --help' for usage information\n";
        std::exit(1);
    }

    // Parse command-specific options and arguments
    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "--vm") {
            options.engine = Engine::VM;
            i++;
        } else if (arg == "--interp") {
            options.engine = Engine::Interpreter;
            i++;
        } else if (arg == "--debug") {
            options.debug = true;
            i++;
        } else if (arg == "--optimize" || arg == "-O") {
            options.optimize = true;
            i++;
        } else if (arg == "--no-optimize" || arg == "-O0") {
            options.optimize = false;
            i++;
        } else if (arg == "--memory-stats") {
            options.memoryStats = true;
            i++;
        } else if (arg == "-o" && (options.command == Command::Compile || options.command == Command::Chunk)) {
            // Output file for compile or chunk command
            if (i + 1 < argc) {
                options.output = argv[i + 1];
                i += 2;
            } else {
                std::cerr << "Error: -o option requires an output filename\n";
                std::exit(1);
            }
        } else if (arg == "--help" || arg == "-h") {
            printCommandHelp(options.command);
            options.command = Command::Help;
            return options;
        } else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option: " << arg << "\n";

            // Map command to string for error message
            std::string cmdName;
            switch (options.command) {
                case Command::Run:
                    cmdName = "run";
                    break;
                case Command::Build:
                    cmdName = "build";
                    break;
                case Command::Check:
                    cmdName = "check";
                    break;
                case Command::Compile:
                    cmdName = "compile";
                    break;
                case Command::Test:
                    cmdName = "test";
                    break;
                case Command::Repl:
                    cmdName = "repl";
                    break;
                case Command::Fmt:
                    cmdName = "fmt";
                    break;
                default:
                    cmdName = "";
                    break;
            }

            if (!cmdName.empty()) {
                std::cerr << "Use 'izi help " << cmdName << "' for usage information\n";
            }
            std::exit(1);
        } else {
            // This is a positional argument
            if (options.command == Command::Run || options.command == Command::Build ||
                options.command == Command::Check || options.command == Command::Compile ||
                options.command == Command::Chunk || options.command == Command::Bench ||
                options.command == Command::Fmt) {
                // These commands expect a filename
                if (options.input.empty()) {
                    options.input = arg;
                } else {
                    // For run command, additional arguments are passed to the script
                    if (options.command == Command::Run) {
                        options.args.push_back(arg);
                    } else {
                        std::cerr << "Error: Unexpected argument: " << arg << "\n";
                        std::exit(1);
                    }
                }
            } else if (options.command == Command::Test) {
                // Test command can have optional pattern arguments
                options.args.push_back(arg);
            }
            i++;
        }
    }

    // Validate command-specific requirements
    if (options.command == Command::Run || options.command == Command::Build || options.command == Command::Check ||
        options.command == Command::Compile || options.command == Command::Chunk || options.command == Command::Bench) {
        if (options.input.empty()) {
            // Map command to string for error message
            std::string cmdName;
            switch (options.command) {
                case Command::Run:
                    cmdName = "run";
                    break;
                case Command::Build:
                    cmdName = "build";
                    break;
                case Command::Check:
                    cmdName = "check";
                    break;
                case Command::Compile:
                    cmdName = "compile";
                    break;
                case Command::Chunk:
                    cmdName = "chunk";
                    break;
                case Command::Bench:
                    cmdName = "bench";
                    break;
                default:
                    cmdName = "";
                    break;
            }

            std::cerr << "Error: No input file specified\n";
            if (!cmdName.empty()) {
                std::cerr << "Use 'izi help " << cmdName << "' for usage information\n";
            }
            std::exit(1);
        }
    }

    if (options.command == Command::Fmt) {
        if (options.input.empty()) {
            std::cerr << "Error: 'fmt' command not yet implemented\n";
            std::cerr << "This feature is planned for v0.2\n";
            std::exit(1);
        }
    }

    return options;
}

}  // namespace izi

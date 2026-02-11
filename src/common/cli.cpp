#include "common/cli.hpp"
#include <iostream>
#include <cstring>

namespace izi {

void CliOptions::printVersion() {
    std::cout << IZILANG_VERSION << "\n";
}

void CliOptions::printHelp() {
    std::cout << "Usage: izi [options] [file]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --version           Show version information\n";
    std::cout << "  --help              Show this help message\n";
    std::cout << "  --vm                Run with bytecode VM (default: tree-walker)\n";
    std::cout << "  --interp            Run with tree-walker interpreter (default)\n";
    std::cout << "  -c <code>           Execute inline code\n";
    std::cout << "  --debug             Enable debug/verbose output\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  izi                 Start interactive REPL\n";
    std::cout << "  izi script.iz       Run a script file\n";
    std::cout << "  izi --vm script.iz  Run with bytecode VM\n";
    std::cout << "  izi -c \"print(42)\"  Execute inline code\n";
}

CliOptions CliOptions::parse(int argc, char** argv) {
    CliOptions options;
    
    // No arguments -> REPL mode
    if (argc == 1) {
        options.mode = ExecutionMode::Repl;
        return options;
    }

    int i = 1;
    bool hasFile = false;
    bool hasInline = false;
    
    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "--version") {
            options.mode = ExecutionMode::Version;
            return options;
        } 
        else if (arg == "--help" || arg == "-h") {
            options.mode = ExecutionMode::Help;
            return options;
        }
        else if (arg == "--vm") {
            options.engine = Engine::VM;
            i++;
        }
        else if (arg == "--interp") {
            options.engine = Engine::Interpreter;
            i++;
        }
        else if (arg == "--debug") {
            options.debug = true;
            i++;
        }
        else if (arg == "-c") {
            if (i + 1 >= argc) {
                std::cerr << "Error: -c requires a code string argument\n";
                std::exit(1);
            }
            options.mode = ExecutionMode::Inline;
            options.input = argv[i + 1];
            hasInline = true;
            i += 2;
        }
        else if (arg[0] == '-') {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information\n";
            std::exit(1);
        }
        else {
            // This is a filename
            options.mode = ExecutionMode::File;
            options.input = arg;
            hasFile = true;
            i++;
        }
    }

    // If no file or inline code provided, default to REPL
    if (!hasFile && !hasInline) {
        options.mode = ExecutionMode::Repl;
    }

    return options;
}

} // namespace izi

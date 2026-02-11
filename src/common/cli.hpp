#pragma once

#include <string>
#include <vector>
#include <optional>

namespace izi {

struct CliOptions {
    enum class ExecutionMode {
        File,         // Execute a file
        Inline,       // Execute inline code with -c
        Repl,         // Interactive REPL mode
        Version,      // Show version
        Help          // Show help
    };

    enum class Engine {
        Interpreter,  // Tree-walker interpreter
        VM            // Bytecode VM
    };

    ExecutionMode mode = ExecutionMode::File;
    Engine engine = Engine::Interpreter;
    bool debug = false;
    std::string input;  // Filename or inline code

    static CliOptions parse(int argc, char** argv);
    static void printVersion();
    static void printHelp();
};

} // namespace izi

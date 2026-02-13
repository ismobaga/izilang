#pragma once

#include <string>
#include <vector>
#include <optional>

namespace izi {

// Version constant
constexpr const char* IZILANG_VERSION = "IziLang 0.2.0";

struct CliOptions {
    enum class Command {
        Run,          // Execute a file
        Build,        // Compile/check only, no execution
        Check,        // Parse + analyze, no execution
        Compile,      // Compile to native executable
        Chunk,        // Compile to bytecode chunk (.izb)
        Test,         // Execute test files
        Repl,         // Interactive REPL mode
        Fmt,          // Format code
        Version,      // Show version
        Help          // Show help
    };

    enum class Engine {
        Interpreter,  // Tree-walker interpreter
        VM            // Bytecode VM
    };

    Command command = Command::Run;
    Engine engine = Engine::Interpreter;
    bool debug = false;
    bool optimize = true;  // Enable optimizations by default
    std::string input;  // Filename or inline code
    std::string output; // Output filename for compile command
    std::vector<std::string> args;  // Additional arguments (e.g., test patterns)

    static CliOptions parse(int argc, char** argv);
    static void printVersion();
    static void printHelp();
    static void printCommandHelp(Command cmd);
};

} // namespace izi

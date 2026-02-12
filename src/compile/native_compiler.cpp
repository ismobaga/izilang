#include "compile/native_compiler.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "common/error_reporter.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <unistd.h> // for getpid()

namespace fs = std::filesystem;

namespace izi {

std::string NativeCompiler::escapeString(const std::string& str) {
    std::string escaped;
    escaped.reserve(str.size() * 1.2); // Reserve some extra space
    
    for (char c : str) {
        switch (c) {
            case '\\': escaped += "\\\\"; break;
            case '"':  escaped += "\\\""; break;
            case '\n': escaped += "\\n"; break;
            case '\r': escaped += "\\r"; break;
            case '\t': escaped += "\\t"; break;
            default:   escaped += c; break;
        }
    }
    
    return escaped;
}

bool NativeCompiler::generateEmbeddedSource(const std::string& sourceCode, const std::string& outputPath) {
    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "Error: Cannot create generated source file: " << outputPath << "\n";
        return false;
    }

    // Generate C++ code that embeds the IziLang source and runs it
    out << "// Auto-generated file - do not edit\n";
    out << "#include <iostream>\n";
    out << "#include <sstream>\n";
    out << "#include <unordered_set>\n";
    out << "\n";
    out << "#include \"interp/interpreter.hpp\"\n";
    out << "#include \"parse/lexer.hpp\"\n";
    out << "#include \"parse/parser.hpp\"\n";
    out << "#include \"compile/compiler.hpp\"\n";
    out << "#include \"bytecode/vm.hpp\"\n";
    out << "#include \"bytecode/vm_native.hpp\"\n";
    out << "#include \"common/error_reporter.hpp\"\n";
    out << "\n";
    out << "using namespace izi;\n";
    out << "\n";
    out << "// Embedded source code\n";
    out << "static const char* EMBEDDED_SOURCE = R\"IZILANGSRC(\n";
    out << sourceCode;
    out << ")IZILANGSRC\";\n";
    out << "\n";
    out << "int main(int argc, char** argv) {\n";
    out << "    std::string src = EMBEDDED_SOURCE;\n";
    out << "    \n";
    out << "    try {\n";
    out << "        Lexer lex(src);\n";
    out << "        auto tokens = lex.scanTokens();\n";
    out << "        Parser parser(std::move(tokens), src);\n";
    out << "        auto program = parser.parse();\n";
    out << "        \n";
    out << "        // Use tree-walker interpreter by default\n";
    out << "        Interpreter interp(src);\n";
    out << "        interp.interpret(program);\n";
    out << "    } catch (const LexerError& e) {\n";
    out << "        ErrorReporter reporter(src);\n";
    out << "        std::cerr << reporter.formatError(e.line, e.column, e.what(), \"Lexer Error\") << '\\n';\n";
    out << "        return 1;\n";
    out << "    } catch (const ParserError& e) {\n";
    out << "        ErrorReporter reporter(src);\n";
    out << "        std::cerr << reporter.formatError(e.token, e.what(), \"Parse Error\") << '\\n';\n";
    out << "        return 1;\n";
    out << "    } catch (const RuntimeError& e) {\n";
    out << "        ErrorReporter reporter(src);\n";
    out << "        std::cerr << reporter.formatError(e.token, e.what()) << '\\n';\n";
    out << "        return 1;\n";
    out << "    } catch (const ThrowSignal& e) {\n";
    out << "        ErrorReporter reporter(src);\n";
    out << "        std::cerr << reporter.formatError(e.token, \"Uncaught exception\") << '\\n';\n";
    out << "        std::cerr << \"Exception value: \";\n";
    out << "        printValue(e.exception);\n";
    out << "        std::cerr << '\\n';\n";
    out << "        return 1;\n";
    out << "    } catch (const std::exception& e) {\n";
    out << "        std::cerr << \"Error: \" << e.what() << '\\n';\n";
    out << "        return 1;\n";
    out << "    }\n";
    out << "    \n";
    out << "    return 0;\n";
    out << "}\n";

    out.close();
    return true;
}

std::string NativeCompiler::getCompilerCommand() {
    // Try to detect available compiler
    // Note: Could cache this result for performance, but detection is fast enough
    if (std::system("which g++ > /dev/null 2>&1") == 0) {
        return "g++";
    } else if (std::system("which clang++ > /dev/null 2>&1") == 0) {
        return "clang++";
    } else if (std::system("which c++ > /dev/null 2>&1") == 0) {
        return "c++";
    }
    
    // Default to g++ and let compilation fail with a clear error
    return "g++";
}

std::vector<std::string> NativeCompiler::getSourceFiles() {
    // Get the source directory path relative to where the izi binary is
    std::vector<std::string> sources = {
        "src/parse/lexer.cpp",
        "src/parse/parser.cpp",
        "src/interp/interpreter.cpp",
        "src/interp/native.cpp",
        "src/interp/user_function.cpp",
        "src/interp/izi_class.cpp",
        "src/interp/native_modules.cpp",
        "src/compile/compiler.cpp",
        "src/bytecode/vm.cpp",
        "src/bytecode/vm_native.cpp",
        "src/bytecode/vm_user_function.cpp",
        "src/bytecode/vm_class.cpp",
        "src/common/error_reporter.cpp",
        "src/common/value.cpp",
        "src/common/semantic_analyzer.cpp",
        "src/ast/type.cpp"
    };
    
    return sources;
}

bool NativeCompiler::compileToExecutable(const std::string& cppFile, const CompileOptions& options) {
    std::string compiler = getCompilerCommand();
    
    if (options.verbose) {
        std::cout << "Using compiler: " << compiler << "\n";
    }

    // Build the compilation command
    std::stringstream cmd;
    cmd << compiler << " ";
    cmd << "-std=c++20 ";
    
    // Add optimization flags
    if (options.debug) {
        cmd << "-g -O0 ";
    } else {
        cmd << "-O2 ";
    }
    
    // Static linking flags
    // Note: Full static linking may not work on all platforms (e.g., macOS)
    // The -static flag will cause compilation to fail on unsupported platforms
    // with a clear error message from the compiler
    cmd << "-static ";
    
    // Include directories
    cmd << "-I. -Isrc ";
    
    // Add the generated source file
    cmd << cppFile << " ";
    
    // Add all source files
    auto sources = getSourceFiles();
    for (const auto& src : sources) {
        cmd << src << " ";
    }
    
    // Output file
    cmd << "-o " << options.outputFile << " ";
    
    // Link libraries statically
    cmd << "-lm -lpthread ";
    
    // Suppress warnings for a cleaner build
    cmd << "2>&1";
    
    if (options.verbose) {
        std::cout << "Compilation command:\n" << cmd.str() << "\n\n";
    } else {
        std::cout << "Compiling to native executable...\n";
    }
    
    // Execute the compilation
    int result = std::system(cmd.str().c_str());
    
    if (result == 0) {
        std::cout << "Successfully compiled to: " << options.outputFile << "\n";
        return true;
    } else {
        std::cerr << "Error: Compilation failed with exit code " << result << "\n";
        std::cerr << "Note: Static linking may not be supported on your platform.\n";
        return false;
    }
}

bool NativeCompiler::compile(const CompileOptions& options) {
    // Validate input file exists
    if (!fs::exists(options.inputFile)) {
        std::cerr << "Error: Input file does not exist: " << options.inputFile << "\n";
        return false;
    }

    // Read the source code
    std::ifstream inFile(options.inputFile);
    if (!inFile.is_open()) {
        std::cerr << "Error: Cannot open input file: " << options.inputFile << "\n";
        return false;
    }
    
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string sourceCode = buffer.str();
    inFile.close();

    // Validate the source code by parsing it
    try {
        if (options.verbose) {
            std::cout << "Validating source code...\n";
        }
        
        Lexer lex(sourceCode);
        auto tokens = lex.scanTokens();
        Parser parser(std::move(tokens), sourceCode);
        auto program = parser.parse();
        
        if (options.verbose) {
            std::cout << "Source code validated successfully.\n";
        }
    } catch (const LexerError& e) {
        ErrorReporter reporter(sourceCode);
        std::cerr << "Lexer error in source file:\n";
        std::cerr << reporter.formatError(e.line, e.column, e.what(), "Lexer Error") << '\n';
        return false;
    } catch (const ParserError& e) {
        ErrorReporter reporter(sourceCode);
        std::cerr << "Parse error in source file:\n";
        std::cerr << reporter.formatError(e.token, e.what(), "Parse Error") << '\n';
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error validating source: " << e.what() << "\n";
        return false;
    }

    // Create temporary directory for generated files
    // Use a combination of timestamp and process ID for uniqueness
    std::stringstream tempDirName;
    tempDirName << "izilang_compile_" << std::time(nullptr) << "_" << getpid();
    fs::path tempDir = fs::temp_directory_path() / tempDirName.str();
    fs::create_directories(tempDir);
    
    if (options.verbose) {
        std::cout << "Using temporary directory: " << tempDir << "\n";
    }

    // Generate embedded source file
    fs::path genCppFile = tempDir / "embedded_main.cpp";
    if (options.verbose) {
        std::cout << "Generating embedded source file...\n";
    }
    
    if (!generateEmbeddedSource(sourceCode, genCppFile.string())) {
        fs::remove_all(tempDir);
        return false;
    }

    // Compile to executable
    bool success = compileToExecutable(genCppFile.string(), options);

    // Clean up temporary files
    if (!options.debug) {
        fs::remove_all(tempDir);
    } else if (options.verbose) {
        std::cout << "Keeping temporary files in: " << tempDir << "\n";
    }

    return success;
}

} // namespace izi

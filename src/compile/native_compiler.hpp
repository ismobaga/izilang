#pragma once

#include <string>
#include <vector>

namespace izi {

/**
 * NativeCompiler - Compiles IziLang source code to native executables
 * 
 * This compiler generates a standalone executable by:
 * 1. Embedding the source code into a C++ file
 * 2. Compiling that C++ file with the IziLang interpreter
 * 3. Statically linking all dependencies
 */
class NativeCompiler {
public:
    struct CompileOptions {
        std::string inputFile;       // Input .iz source file
        std::string outputFile;      // Output executable name
        bool debug = false;          // Include debug symbols
        bool verbose = false;        // Print compilation steps
    };

    /**
     * Compile an IziLang source file to a native executable
     * @param options Compilation options
     * @return true if successful, false otherwise
     */
    static bool compile(const CompileOptions& options);

private:
    /**
     * Generate C++ code that embeds the IziLang source
     * @param sourceCode The IziLang source code to embed
     * @param outputPath Path to write the generated C++ file
     * @return true if successful
     */
    static bool generateEmbeddedSource(const std::string& sourceCode, const std::string& outputPath);

    /**
     * Compile the generated C++ file to a native executable
     * @param cppFile Path to the generated C++ file
     * @param options Compilation options
     * @return true if successful
     */
    static bool compileToExecutable(const std::string& cppFile, const CompileOptions& options);

    /**
     * Escape a string for C++ string literal
     * @param str String to escape
     * @return Escaped string
     */
    static std::string escapeString(const std::string& str);

    /**
     * Get the compiler command for the current platform
     * @return Compiler command (e.g., "g++", "clang++")
     */
    static std::string getCompilerCommand();

    /**
     * Get source files needed for compilation
     * @return List of source file paths
     */
    static std::vector<std::string> getSourceFiles();
};

} // namespace izi

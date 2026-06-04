#pragma once

#include "bytecode/vm.hpp"
#include "bytecode/vm_native.hpp"
#include "compile/compiler.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace izi::testutil {

class OutputCapture {
   public:
    OutputCapture() : oldBuf_(std::cout.rdbuf(buffer_.rdbuf())) {}
    ~OutputCapture() { std::cout.rdbuf(oldBuf_); }

    std::string output() const { return buffer_.str(); }

   private:
    std::stringstream buffer_;
    std::streambuf* oldBuf_;
};

inline std::vector<StmtPtr> parseProgram(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    return parser.parse();
}

inline Chunk compileVmChunk(const std::string& source) {
    auto program = parseProgram(source);
    BytecodeCompiler compiler;
    return compiler.compile(program);
}

inline Value runVmSource(const std::string& source, bool registerNatives = true) {
    Chunk chunk = compileVmChunk(source);
    VM vm;
    if (registerNatives) {
        registerVmNatives(vm);
    }
    return vm.run(chunk);
}

inline std::string runVmSourceAndCapture(const std::string& source, bool registerNatives = true) {
    OutputCapture capture;
    (void)runVmSource(source, registerNatives);
    return capture.output();
}

}  // namespace izi::testutil

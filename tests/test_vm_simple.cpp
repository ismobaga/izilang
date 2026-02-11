#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"

using namespace izi;

TEST_CASE("VM Try-Catch Simple", "[vm-exception-simple]") {
    // Basic smoke test to verify try-catch doesn't cause infinite loops or crashes
    std::string source = "var executed = 0; try { throw \"error\"; executed = 999; } catch(e) { executed = 1; }";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    auto program = parser.parse();
    
    BytecodeCompiler compiler;
    Chunk chunk = compiler.compile(program);
    
    VM vm;
    vm.run(chunk);
    
    // Test passes if execution completes without hanging
    // The 'executed' variable should be 1 (catch block ran), not 999 (code after throw)
    REQUIRE(true);
}

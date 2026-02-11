#include "catch.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include "compile/compiler.hpp"
#include "bytecode/vm.hpp"

using namespace izi;

TEST_CASE("VM Try-Catch Simple", "[vm-exception-simple]") {
    std::string source = "try { throw \"error\"; } catch(e) { }";
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens), source);
    auto program = parser.parse();
    
    BytecodeCompiler compiler;
    Chunk chunk = compiler.compile(program);
    
    VM vm;
    vm.run(chunk);
    
    REQUIRE(true);  // Just checking it doesn't hang
}

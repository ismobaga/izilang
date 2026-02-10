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

using namespace izi;

int main(int argc, char** argv) {
    std::string src;

    if (argc > 1) {
        std::ifstream f(argv[1]);
        if (!f.is_open()) {
            std::cerr << "Cannot open file: " << argv[1] << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << f.rdbuf();
        src = buffer.str();
    } else {
        std::stringstream buffer;
        buffer << std::cin.rdbuf();
        src = buffer.str();
    }

    try {
        Lexer lex(src);
        auto tokens = lex.scanTokens();
        Parser parser(std::move(tokens), src);
        auto program = parser.parse();
        bool useVM = false;

        if (!useVM)
        {
            
            
            Interpreter interp(src);
            interp.interpret(program);
        }
        else
        {
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
        return 1;
    } catch (const ParserError& e) {
        ErrorReporter reporter(src);
        std::cerr << reporter.formatError(e.token, e.what(), "Parse Error") << '\n';
        return 1;
    } catch (const RuntimeError& e) {
        ErrorReporter reporter(src);
        std::cerr << reporter.formatError(e.token, e.what()) << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
};
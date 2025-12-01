#include <fstream>
#include <iostream>
#include <sstream>

#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"

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
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp;
        interp.interpret(program);
    } catch (const std::exception& e) {
        std::cerr << "Error : " << e.what() << '\n';
        return 1;
    }

    return 0;
};
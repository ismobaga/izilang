#include <iostream>
#include <string>
#include "lsp_server.hpp"

int main(int argc, char* argv[]) {
    // Check for --stdio flag
    bool stdio_mode = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--stdio") {
            stdio_mode = true;
            break;
        }
    }

    if (!stdio_mode) {
        std::cerr << "IziLang LSP Server v0.1.0\n";
        std::cerr << "Usage: izilang-lsp --stdio\n";
        std::cerr << "\nThis is a Language Server Protocol implementation for IziLang.\n";
        std::cerr << "Run with --stdio to communicate over standard input/output.\n";
        return 1;
    }

    izi::lsp::LSPServer server;
    server.run();
    
    return 0;
}

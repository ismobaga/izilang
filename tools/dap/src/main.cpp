#include <iostream>
#include <string>
#include "dap_server.hpp"

int main(int argc, char* argv[]) {
    bool stdioMode = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--stdio") {
            stdioMode = true;
            break;
        }
    }

    if (!stdioMode) {
        std::cerr << "IziLang DAP Server v0.1.0\n";
        std::cerr << "Usage: izilang-dap --stdio\n";
        std::cerr << "\nThis is a Debug Adapter Protocol implementation for IziLang.\n";
        std::cerr << "Run with --stdio to communicate over standard input/output.\n";
        return 1;
    }

    izi::dap::DAPServer server;
    server.run();

    return 0;
}

#include "catch.hpp"
#include "interp/interpreter.hpp"
#include "parse/lexer.hpp"
#include "parse/parser.hpp"
#include <filesystem>
#include <fstream>

using namespace izi;
namespace fs = std::filesystem;

// Helper to create a temporary test file
class TempFile {
   public:
    std::string path;

    TempFile(const std::string& filename, const std::string& content) {
        path = (fs::temp_directory_path() / filename).string();
        std::ofstream file(path);
        file << content;
        file.close();
    }

    ~TempFile() {
        if (fs::exists(path)) {
            fs::remove(path);
        }
    }
};

TEST_CASE("Relative imports - same directory", "[imports][relative]") {
    SECTION("Import module with ./ prefix") {
        // Create a module file
        TempFile module("test_module_same.izi", "var MODULE_VAR = 42;");

        // Create main file that imports the module
        std::string mainContent = "import \"./test_module_same\";\nvar result = MODULE_VAR;";
        TempFile main("test_main_same.izi", mainContent);

        // Parse and interpret
        std::ifstream file(main.path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        interp.setCurrentFile(main.path);
        REQUIRE_NOTHROW(interp.interpret(program));
    }
}

TEST_CASE("Relative imports - subdirectory", "[imports][relative]") {
    SECTION("Import module from subdirectory") {
        // Create directory structure
        fs::path tempDir = fs::temp_directory_path() / "test_subdir_imports";
        fs::create_directories(tempDir / "lib");

        // Create a module in lib/
        std::string modulePath = (tempDir / "lib" / "helper.izi").string();
        std::ofstream moduleFile(modulePath);
        moduleFile << "var HELPER_VALUE = 100;";
        moduleFile.close();

        // Create main file that imports from lib/
        std::string mainPath = (tempDir / "main.izi").string();
        std::ofstream mainFile(mainPath);
        mainFile << "import \"./lib/helper\";\nvar result = HELPER_VALUE;";
        mainFile.close();

        // Parse and interpret
        std::ifstream file(mainPath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        interp.setCurrentFile(mainPath);
        REQUIRE_NOTHROW(interp.interpret(program));

        // Cleanup
        fs::remove_all(tempDir);
    }
}

TEST_CASE("Relative imports - parent directory", "[imports][relative]") {
    SECTION("Import module from parent directory") {
        // Create directory structure
        fs::path tempDir = fs::temp_directory_path() / "test_parent_imports";
        fs::create_directories(tempDir / "subdir");

        // Create a module in parent directory
        std::string parentModulePath = (tempDir / "parent_module.izi").string();
        std::ofstream parentFile(parentModulePath);
        parentFile << "var PARENT_VAR = 200;";
        parentFile.close();

        // Create main file in subdir/ that imports from parent
        std::string mainPath = (tempDir / "subdir" / "main.izi").string();
        std::ofstream mainFile(mainPath);
        mainFile << "import \"../parent_module\";\nvar result = PARENT_VAR;";
        mainFile.close();

        // Parse and interpret
        std::ifstream file(mainPath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        interp.setCurrentFile(mainPath);
        REQUIRE_NOTHROW(interp.interpret(program));

        // Cleanup
        fs::remove_all(tempDir);
    }
}

TEST_CASE("Nested relative imports", "[imports][relative][nested]") {
    SECTION("A imports B, B imports C with relative paths") {
        // Create directory structure
        fs::path tempDir = fs::temp_directory_path() / "test_nested_imports";
        fs::create_directories(tempDir / "lib");

        // Create module C (base)
        std::string moduleC = (tempDir / "lib" / "base.izi").string();
        std::ofstream fileC(moduleC);
        fileC << "var BASE = 10;";
        fileC.close();

        // Create module B (imports C)
        std::string moduleB = (tempDir / "lib" / "middle.izi").string();
        std::ofstream fileB(moduleB);
        fileB << "import \"./base\";\nvar MIDDLE = BASE + 5;";
        fileB.close();

        // Create main file A (imports B)
        std::string mainPath = (tempDir / "main.izi").string();
        std::ofstream mainFile(mainPath);
        mainFile << "import \"./lib/middle\";\nvar result = MIDDLE;";
        mainFile.close();

        // Parse and interpret
        std::ifstream file(mainPath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        interp.setCurrentFile(mainPath);
        REQUIRE_NOTHROW(interp.interpret(program));

        // Cleanup
        fs::remove_all(tempDir);
    }
}

TEST_CASE("Circular import detection", "[imports][circular]") {
    SECTION("Direct circular import A -> B -> A") {
        // Create directory structure
        fs::path tempDir = fs::temp_directory_path() / "test_circular_imports";
        fs::create_directories(tempDir);

        // Create module A (imports B)
        std::string moduleA = (tempDir / "module_a.izi").string();
        std::ofstream fileA(moduleA);
        fileA << "import \"./module_b\";\nvar A = 1;";
        fileA.close();

        // Create module B (imports A - circular!)
        std::string moduleB = (tempDir / "module_b.izi").string();
        std::ofstream fileB(moduleB);
        fileB << "import \"./module_a\";\nvar B = 2;";
        fileB.close();

        // Create main file that imports A
        std::string mainPath = (tempDir / "main.izi").string();
        std::ofstream mainFile(mainPath);
        mainFile << "import \"./module_a\";";
        mainFile.close();

        // Parse and interpret - should throw on circular import
        std::ifstream file(mainPath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        interp.setCurrentFile(mainPath);

        // Should throw a runtime_error with "Circular import detected"
        bool threw = false;
        std::string errorMsg;
        try {
            interp.interpret(program);
        } catch (const std::runtime_error& e) {
            threw = true;
            errorMsg = e.what();
        }

        REQUIRE(threw);
        REQUIRE(errorMsg.find("Circular import detected") != std::string::npos);

        // Cleanup
        fs::remove_all(tempDir);
    }
}

TEST_CASE("Module deduplication with relative paths", "[imports][deduplication]") {
    SECTION("Same module imported multiple times is only loaded once") {
        // Create directory structure
        fs::path tempDir = fs::temp_directory_path() / "test_dedup_imports";
        fs::create_directories(tempDir);

        // Create a shared module
        std::string sharedModule = (tempDir / "shared.izi").string();
        std::ofstream fileShared(sharedModule);
        fileShared << "var SHARED = 99;";
        fileShared.close();

        // Create main file that imports shared twice
        std::string mainPath = (tempDir / "main.izi").string();
        std::ofstream mainFile(mainPath);
        mainFile << "import \"./shared\";\nimport \"./shared\";\nvar result = SHARED;";
        mainFile.close();

        // Parse and interpret - should not fail
        std::ifstream file(mainPath);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        Parser parser(std::move(tokens));
        auto program = parser.parse();

        Interpreter interp(source);
        interp.setCurrentFile(mainPath);
        REQUIRE_NOTHROW(interp.interpret(program));

        // Cleanup
        fs::remove_all(tempDir);
    }
}

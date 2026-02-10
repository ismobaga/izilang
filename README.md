# IziLang 

A programming language interpreter and bytecode virtual machine implemented in modern C++20.

[![Build Status](https://img.shields.io/badge/build-failing-red)](CODEBASE_ANALYSIS.md)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

> ⚠️ **Project Status**: Currently **does not compile** due to critical issues. See [PROBLEMS.md](PROBLEMS.md) for details.

## Overview

IziLang is a dynamically-typed programming language featuring:
- Variables and basic arithmetic
- Functions with parameters and return values
- Control flow (if/else, while, for loops)
- Collections (arrays and maps)
- Module import system
- Dual execution modes: tree-walking interpreter and bytecode VM

## Quick Start

### Prerequisites
- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- Premake5 ([download](https://premake.github.io/download))
- Make (on Linux/macOS)

### Build Instructions

⚠️ **Note**: Project currently fails to build. See [Fixing Build Issues](#fixing-build-issues) below.

```bash
# Generate build files
premake5 gmake2

# Build (will fail currently)
make config=debug

# Run (after fixing build issues)
./bin/Debug/izi test.iz
```

### Fixing Build Issues

The project has **3 critical build issues** that must be fixed first:

1. **Syntax error in vm.cpp line 40**
2. **Duplicate declarations in vm.hpp**
3. **Missing Chunk::addName() method**

See [PROBLEMS.md](PROBLEMS.md#-critical-blocks-compilation) for detailed fixes.

## Language Examples

### Variables and Arithmetic
```javascript
var x = 10;
var y = 5;
var sum = x + y;
print(sum);  // 15
```

### Functions
```javascript
fn add(a, b) {
    return a + b;
}

var result = add(3, 7);
print(result);  // 10
```

### Control Flow
```javascript
if (x > y) {
    print("x is greater");
} else {
    print("y is greater");
}

var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
```

### Arrays and Maps
```javascript
var numbers = [1, 2, 3, 4, 5];
numbers[0] = 10;
print(numbers);  // [10, 2, 3, 4, 5]

var person = {"name": "Alice", "age": 30};
person["city"] = "Boston";
print(person);  // {name: Alice, age: 30, city: Boston}
```

### Modules
```javascript
// mod.iz
fn add(a, b) {
    return a + b;
}

// main.iz
import "mod.iz";
print(add(2, 3));  // 5
```

## Documentation

- **[CODEBASE_ANALYSIS.md](CODEBASE_ANALYSIS.md)** - Comprehensive technical analysis
- **[PROBLEMS.md](PROBLEMS.md)** - Complete list of issues and bugs
- **[FEATURES.md](FEATURES.md)** - Planned features and roadmap
- **[.github/copilot-instructions.md](.github/copilot-instructions.md)** - Architecture guide

## Project Structure

```
izilang/
├── src/
│   ├── ast/           # Abstract Syntax Tree definitions
│   ├── bytecode/      # Bytecode VM implementation
│   ├── common/        # Shared types (Token, Value, etc.)
│   ├── compile/       # Bytecode compiler
│   ├── interp/        # Tree-walking interpreter
│   ├── parse/         # Lexer and parser
│   └── main.cpp       # Entry point
├── test.iz            # Test script
└── premake5.lua       # Build configuration
```

## Current Status

### ❌ Not Working
- **Compilation** - Critical build errors
- **Bytecode VM** - Functions and imports not implemented
- **Logic operators** - `and`/`or` not supported

### ⚠️ Partially Working
- **Arrays/Maps** - Work in interpreter, incomplete in VM
- **Functions** - Work in interpreter, crash in VM
- **Imports** - Work in interpreter, not in VM

### ✅ Working (Interpreter Only)
- Basic arithmetic and variables
- If/else statements
- While loops
- Print statements
- Native functions (len, clock)

See [CODEBASE_ANALYSIS.md](CODEBASE_ANALYSIS.md) for full feature status.

## Development Roadmap

### Phase 1: Critical Fixes (Week 1)
- [ ] Fix compilation errors
- [ ] Complete VM stack operations
- [ ] Add comprehensive test suite

### Phase 2: VM Completion (Weeks 2-3)
- [ ] Implement array/map opcodes
- [ ] Add local variable support
- [ ] Implement logic operators

### Phase 3: Feature Parity (Weeks 4-6)
- [ ] Function compilation for VM
- [ ] Import system for VM
- [ ] Break/continue statements

### Phase 4: Enhancements (Months 2-3)
- [ ] Standard library
- [ ] Error handling (try-catch)
- [ ] REPL mode
- [ ] Better CLI

See [FEATURES.md](FEATURES.md) for complete roadmap.

## Contributing

Contributions are welcome! Before contributing:

1. Read [CODEBASE_ANALYSIS.md](CODEBASE_ANALYSIS.md) to understand the architecture
2. Check [PROBLEMS.md](PROBLEMS.md) for known issues
3. See [FEATURES.md](FEATURES.md) for planned features
4. Open an issue to discuss your changes

### Good First Issues
- Fix critical build errors ([PROBLEMS.md](PROBLEMS.md#-critical-blocks-compilation))
- Add division by zero checks
- Remove unused includes/fields
- Write basic test cases

## Architecture

IziLang uses the **Visitor pattern** for AST traversal:

```cpp
// Expression types implement accept()
struct BinaryExpr : Expr {
    Value accept(ExprVisitor& v) override { 
        return v.visit(*this); 
    }
};

// Visitors implement visit() for each node
class Interpreter : public ExprVisitor {
    Value visit(BinaryExpr& expr) override {
        // Interpret the expression
    }
};
```

Runtime values use `std::variant`:
```cpp
using Value = std::variant<
    Nil, bool, double, std::string,
    std::shared_ptr<Array>,
    std::shared_ptr<Map>,
    std::shared_ptr<Callable>
>;
```

## Performance

*Performance benchmarks coming after VM completion.*

## License

This project is open source. License TBD.

## Acknowledgments

- Inspired by [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom
- Built with modern C++20 features

## Contact

- **Repository**: [ismobaga/izilang](https://github.com/ismobaga/izilang)
- **Issues**: [GitHub Issues](https://github.com/ismobaga/izilang/issues)

---

**Note**: This project is in active development. Many features are incomplete or not working. See documentation for current status.

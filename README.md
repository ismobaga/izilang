

# IziLang

**Version 0.4.0-dev** - A modern, expressive programming language with excellent tooling and developer experience.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/ismobaga/izilang/actions)

## Features

### 🚀 Professional CLI Toolchain

IziLang provides a complete command-line interface for modern development workflows:

```bash
izi run script.iz      # Execute code
izi build app.iz       # Compile without running
izi check src/*.iz     # Fast syntax validation
izi compile app.iz     # Compile to native executable
izi test               # Run test suite
izi repl               # Interactive REPL
```

Supported commands and current implementation notes are tracked in [docs/IMPLEMENTATION_STATUS.md](docs/IMPLEMENTATION_STATUS.md).

### 🎯 Excellent Error Messages

Clear, actionable error messages with file location, line numbers, and visual indicators:

```
In file 'script.iz':
Runtime Error at line 3, column 10:
  3 | var z = x + y;
    |          ^
Cannot add number and string. Operands must be two numbers or two strings.
```

**[→ See implementation status](docs/IMPLEMENTATION_STATUS.md)**

### 💻 Interactive REPL

Powerful REPL with multi-line input, special commands, and error recovery:

```
IziLang 0.4.0-dev REPL
> fn add(x, y) {
... return x + y;
... }
> print(add(5, 3));
8
> :help    # Show available commands
```

For the current runtime and tooling surface, see [docs/IMPLEMENTATION_STATUS.md](docs/IMPLEMENTATION_STATUS.md).

### 🔧 Rich Language Features

- **Variables & Functions** - First-class functions, closures
- **Collections** - Arrays, maps, sets with rich operations
- **Pattern Matching** - Expressive match expressions
- **Exception Handling** - try/catch/finally blocks
- **Module System** - Import/export for code organization
- **String Interpolation** - Template literals with expressions
- **Classes & OOP** - Classes with inheritance, constructors, `this` binding
- **Gradual Typing** - Optional type annotations on variables and functions
- **Semantic Analysis** - Static checks via `izi check` (unused vars, dead code)
- **Async/Await** - Asynchronous function syntax (`async fn` / `await`); full runtime event loop is planned
- **Macros** - Compile-time macro definitions and expansion (`name!(args)`) — experimental
- **Nullish Coalescing** - `??` operator returns left if not nil, otherwise right
- **IPC** - Named-pipe inter-process communication via `std.ipc` — experimental

### 📦 Dual Execution Modes

Choose between tree-walker interpreter or bytecode VM:

```bash
izi run --interp script.iz    # Tree-walker (default)
izi run --vm script.iz         # Bytecode VM (faster)
```

### 🔧 Native Compilation

Compile IziLang programs to standalone executables with no runtime dependencies:

```bash
izi compile app.iz -o myapp    # Create standalone executable
./myapp                        # Run without IziLang installed
```

Features:
- **Fully static linking** - No runtime dependencies required
- **Portable executables** - Distribute binaries without IziLang installation
- **Native performance** - Optimized release builds

## Quick Start

### Installation

```bash
git clone https://github.com/ismobaga/izilang.git
cd izilang

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

./build/izi --version
```

#### Legacy Premake build

```bash
./premake5 gmake2
make config=release
```

### Your First Program

Create `hello.iz`:

```izilang
fn greet(name) {
    print("Hello, " + name + "!");
}

greet("World");
```

Run it:

```bash
izi hello.iz
# Output: Hello, World!
```

For setup and usage details, see [docs/IMPLEMENTATION_STATUS.md](docs/IMPLEMENTATION_STATUS.md) and the [Standard Library Reference](docs/stdlib/README.md).

## Documentation

The documentation set in this repository currently focuses on implementation status, VM parity, stdlib coverage, and the browser playground.

### Reference

- **[Implementation Status](docs/IMPLEMENTATION_STATUS.md)** - Source of truth for feature coverage and tooling status
- **[VM Parity Checklist](docs/VM_PARITY_CHECKLIST.md)** - Bytecode VM feature parity tracking
- **[Standard Library Reference](docs/stdlib/README.md)** - Module overview and built-in functions
- **[Web Playground](docs/WEB_PLAYGROUND.md)** - Browser playground architecture and implementation notes

### Module Docs

- **[assert](docs/stdlib/assert.md)** - Assertions for testing and validation
- **[env](docs/stdlib/env.md)** - Environment variable access
- **[io](docs/stdlib/io.md)** - File I/O and console output
- **[http](docs/stdlib/http.md)** - Synchronous HTTP client
- **[json](docs/stdlib/json.md)** - JSON parsing and serialization
- **[math](docs/stdlib/math.md)** - Mathematical constants and functions
- **[string](docs/stdlib/string.md)** - String manipulation functions
- **[time](docs/stdlib/time.md)** - Time, sleep, and formatting
- **[regex](docs/stdlib/regex.md)** - Regular expression matching and replacement
- **[array](docs/stdlib/array.md)** - Array utilities and higher-order functions
- **[ipc](docs/stdlib/ipc.md)** - Inter-process communication via named pipes
- **[image](docs/stdlib/image.md)** - Image loading and processing
- **[audio](docs/stdlib/audio.md)** - Audio playback
- **[ui](docs/stdlib/ui.md)** - Graphical window and drawing support

### Project Notes

- **[Implementation Status](docs/IMPLEMENTATION_STATUS.md)** - Current feature status and planned work
- **[VM Parity Checklist](docs/VM_PARITY_CHECKLIST.md)** - Open VM gaps and runtime differences
- **[Web Playground](docs/WEB_PLAYGROUND.md)** - Frontend roadmap for the browser playground

## Language Examples

### Functions & Closures

```izilang
fn makeCounter() {
    var count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
}

var counter = makeCounter();
print(counter());  // 1
print(counter());  // 2
```

### Pattern Matching

```izilang
fn fizzbuzz(n) {
    match (n % 15, n % 3, n % 5) {
        case (0, _, _) => "FizzBuzz",
        case (_, 0, _) => "Fizz",
        case (_, _, 0) => "Buzz",
        case _ => toString(n)
    }
}
```

### Collections

```izilang
var fruits = ["apple", "banana", "cherry"];
push(fruits, "date");

var person = {
    name: "Alice",
    age: 30,
    greet: fn() { print("Hello!"); }
};
```

### Nullish Coalescing

Use `??` to provide a default when a value might be `nil`:

```izilang
fn findUser(id) {
    // returns nil if not found
}

var user = findUser(42) ?? { name: "Guest", role: "visitor" };
print(user.name);  // "Guest" if not found

// Chains: first non-nil wins
var config = envValue ?? fileValue ?? defaultValue;
```

**[→ More examples in examples/](examples/)**

## Development

### Building from Source

```bash
# Debug build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Release build
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
```

Binary location: `./build/izi` or `./build-release/izi`

#### Legacy (Premake) build flow

```bash
./premake5 gmake2
make config=debug
make config=release
```

### Running Tests

```bash
# Configure and build with tests enabled
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DIZI_BUILD_TESTS=ON
cmake --build build --parallel

# Run C++ unit tests
ctest --test-dir build --output-on-failure

# Run IziLang test files
./build/izi test
```

#### C++ Test Suite

Comprehensive unit and integration tests using Catch2:

```bash
# Run all tests
./build/izilang_tests

# Run specific test categories
./build/izilang_tests [lexer]
./build/izilang_tests [integration]

# List available tests
./build/izilang_tests --list-tests

# Verbose output
./build/izilang_tests -s
```

**Test Coverage:**
- Lexer: Tokenization, operators, keywords, literals
- Parser: Expressions, statements, precedence
- Interpreter: Variables, functions, control flow
- Collections: Arrays, maps, sets
- Pattern matching, exceptions, modules
- VM: Bytecode compilation and execution

## Project Structure

```
izilang/
├── src/              # Source code
│   ├── ast/          # Abstract syntax tree
│   ├── bytecode/     # VM and bytecode compiler
│   ├── common/       # Shared utilities (CLI, errors, values)
│   ├── compile/      # Compilation pipeline
│   ├── interp/       # Tree-walker interpreter
│   └── parse/        # Lexer and parser
├── tests/            # C++ unit tests (Catch2)
├── examples/         # Example IziLang programs
├── docs/             # Documentation
├── tools/            # Additional tools
│   ├── lsp/          # Language Server Protocol (coming soon)
│   ├── pkg/          # Package manager (coming soon)
│   └── vscode-extension/  # VS Code extension
└── std/              # Standard library modules
```

## Contributing

Contributions are welcome through issues and pull requests.

Areas where help is needed:
- Language features and standard library
- Documentation and examples
- Editor extensions and tooling
- Bug fixes and performance improvements

## Roadmap

The detailed feature matrix lives in [docs/IMPLEMENTATION_STATUS.md](docs/IMPLEMENTATION_STATUS.md). The current focus is parity between the tree-walker interpreter and the bytecode VM, plus the stdlib and browser playground work tracked in [docs/VM_PARITY_CHECKLIST.md](docs/VM_PARITY_CHECKLIST.md) and [docs/WEB_PLAYGROUND.md](docs/WEB_PLAYGROUND.md).

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Community & Support

- **Issues**: [GitHub Issues](https://github.com/ismobaga/izilang/issues)
- **Discussions**: [GitHub Discussions](https://github.com/ismobaga/izilang/discussions)
- **Documentation**: [docs/](docs/)

## Acknowledgments

IziLang is inspired by modern languages like Python, JavaScript, and Rust, with a focus on developer ergonomics and clear error messages.

---

**Made with ❤️ by the IziLang team**
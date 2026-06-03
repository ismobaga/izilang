

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

**[→ See CLI Reference](docs/CLI_REFERENCE.md)**

### 🎯 Excellent Error Messages

Clear, actionable error messages with file location, line numbers, and visual indicators:

```
In file 'script.iz':
Runtime Error at line 3, column 10:
  3 | var z = x + y;
    |          ^
Cannot add number and string. Operands must be two numbers or two strings.
```

**[→ See Error Guide](docs/ERRORS_TROUBLESHOOTING.md)**

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

**[→ Getting Started](docs/GETTING_STARTED.md)**

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
# Clone repository
git clone https://github.com/ismobaga/izilang.git
cd izilang

# Build (readline disabled by default for minimal builds)
./premake5 gmake2
make config=release

# Optional: build with readline support (requires libreadline-dev on Linux)
# sudo apt-get install libreadline-dev   # Ubuntu/Debian
# brew install readline                  # macOS
./premake5 gmake2 --readline
make config=release

# Optional: Add to PATH
export PATH="$PATH:$(pwd)/bin/Release/izi"
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

**[→ Complete Getting Started Guide](docs/GETTING_STARTED.md)**

## Documentation

### Learning Resources

- **[Tutorial](docs/TUTORIAL.md)** - Learn IziLang in Y minutes: guided tour with exercises
- **[Getting Started](docs/GETTING_STARTED.md)** - Installation, basics, examples
- **[Cookbook](docs/COOKBOOK.md)** - Practical patterns, best practices, real-world examples

### Reference

- **[Language Specification](docs/LANGUAGE_SPEC.md)** - Formal grammar (EBNF), type system, semantics
- **[Standard Library](docs/STANDARD_LIBRARY.md)** - All built-in modules and functions with examples
- **[CLI Reference](docs/CLI_REFERENCE.md)** - Complete command-line guide

### Guides

- **[IPC Module](docs/IPC.md)** - Inter-process communication via named pipes
- **[Module System](docs/MODULE_SYSTEM.md)** - Import/export and module organisation
- **[Error System](docs/ERROR_SYSTEM.md)** - Error types and handling
- **[Testing](docs/TESTING.md)** - How to run and write tests
- **[Editor Setup](docs/EDITOR_SETUP.md)** - VS Code, Vim, Emacs integration
- **[Troubleshooting](docs/ERRORS_TROUBLESHOOTING.md)** - Common errors and solutions

### Project

- **[Roadmap](docs/ROADMAP.md)** - Development plans and milestones
- **[Analysis](docs/ANALYSIS.md)** - Project analysis, feature inventory, and v1.0 scope

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
# Generate build files
./premake5 gmake2

# Build debug version
make config=debug

# Build release version  
make config=release
```

Binary location: `./bin/Debug/izi/izi` or `./bin/Release/izi/izi`

### Running Tests

```bash
# Build tests
make config=debug

# Run C++ unit tests
./bin/Debug/tests/tests

# Run IziLang test files
izi test
```

#### C++ Test Suite

Comprehensive unit and integration tests using Catch2:

```bash
# Run all tests
./bin/Debug/tests/tests

# Run specific test categories
./bin/Debug/tests/tests [lexer]
./bin/Debug/tests/tests [integration]

# List available tests
./bin/Debug/tests/tests --list-tests

# Verbose output
./bin/Debug/tests/tests -s
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

Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

Areas where help is needed:
- Language features and standard library
- Documentation and examples
- Editor extensions and tooling
- Bug fixes and performance improvements

## Roadmap

**v0.3** (Released) - Language Power & Performance ✅
- ✅ Gradual typing with optional type annotations
- ✅ Classes and OOP with inheritance
- ✅ Mark-and-Sweep garbage collection
- ✅ Semantic analysis and static checks
- ✅ Enhanced stack traces and error reporting
- ✅ VM import system and benchmark command (`izi bench`)
- ✅ Stack overflow protection and memory statistics

**v0.4** (In Progress) - Concurrency & Ecosystem
- ✅ Code formatter (`izi fmt`) — experimental
- ✅ Async/await syntax (parse + partial runtime; event loop planned)
- ✅ Macro system (experimental)
- ✅ Rich stdlib (json, time, regex, http, net, log, ipc) — partial/experimental
- ✅ Nullish coalescing operator (`??`)
- 🚧 LSP server — in progress, not yet stable
- 🚧 Package manager MVP (`izi-pkg`) — planned, not yet implemented
- 🚧 Full async I/O (`std.async` event loop) — planned

**v1.0** (Target Q1 2027) - Stable
- Language freeze (no breaking changes)
- Package registry
- Production deployments

**[→ Full Roadmap](docs/ROADMAP.md)** | **[→ Project Analysis](docs/ANALYSIS.md)** | **[→ Implementation Status](docs/IMPLEMENTATION_STATUS.md)**

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
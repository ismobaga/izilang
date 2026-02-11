# Changelog

All notable changes to IziLang will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-02-11

### Added - Core Language Features
- **Lexer & Parser**: Complete tokenization and parsing with accurate error reporting
- **Data Types**: nil, bool, number (64-bit float), string, array, map
- **Operators**: Arithmetic (+, -, *, /), comparison (<, >, <=, >=, ==, !=), logical (and, or, !)
- **Control Flow**: if/else statements, while loops, for loops, break, continue
- **Functions**: First-class functions with closures and recursion support
- **Pattern Matching**: match expressions with guards and wildcard patterns
- **Exception Handling**: try/catch/finally blocks with stack traces
- **String Interpolation**: f-strings with `${}` syntax for embedding expressions
- **Comments**: Single-line (`//`) and multi-line (`/* */`) comments

### Added - Module System
- **ES6-style imports**: `import { name } from "module"`
- **Wildcard imports**: `import * as name from "module"` creates namespace objects
- **Module exports**: `export var`, `export fn`
- **Native modules**: math, string, array, io (46 built-in functions)
- **Circular dependency handling**: Lazy loading prevents infinite loops

### Added - Standard Library
- **std.math**: sqrt, pow, sin, cos, tan, floor, ceil, round, abs, min, max, PI, E
- **std.string**: substring, split, toUpper, toLower, trim, replace, indexOf, startsWith, endsWith, join
- **std.array**: map, filter, reduce, sort, reverse, concat, slice
- **std.io**: readFile, writeFile, appendFile, fileExists

### Added - Developer Experience
- **Error Messages**: Rust-quality diagnostics with source locations, code snippets, and carets
- **Stack Traces**: Exception traces with function names and line numbers
- **Multi-platform Support**: Linux, macOS, Windows builds via Premake5
- **Fast Build System**: Clean builds in 2-3 seconds
- **Comprehensive Tests**: 68 test cases, 328 assertions, 100% passing

### Added - Execution Modes
- **Tree-Walking Interpreter** (default): Direct AST traversal, excellent error messages
- **Bytecode VM** (experimental): Stack-based VM with 30+ opcodes (opt-in via `--vm` flag)

### Added - Documentation
- Comprehensive STATUS.md with project maturity assessment
- DECISIONS.md documenting all frozen design decisions
- ROADMAP.md for future development plans
- 17 markdown documentation files

### Design Decisions (Frozen for v0.1)
- **Type System**: Fully dynamic typing, no type annotations
- **Memory Management**: Reference counting via std::shared_ptr
- **Error Handling**: Exceptions with try/catch/finally (not Result types)
- **Execution Model**: Hybrid (interpreter default, VM experimental)
- **Module System**: ES6-style with named and wildcard imports
- **Standard Library**: Batteries-included philosophy (46 functions)
- **Backward Compatibility**: Breaking changes allowed pre-v1.0, SemVer after v1.0

### Known Limitations
- Reference counting cannot handle circular references (users should avoid them)
- No static type checking or type inference
- No classes or OOP features (planned for v0.2)
- No concurrency support (planned for v0.3)
- VM mode is experimental and not recommended for production

### Developer Notes
- **Language**: C++20
- **Build Tool**: Premake5
- **Test Framework**: Catch2 v2.13.10
- **Binary Size**: ~1.2 MB (debug), ~400 KB (release)
- **Line Count**: ~2,700 LOC (core implementation)

---

## Future Roadmap

### v0.2 (Planned)
- Classes and OOP features
- Dot notation for module objects (`module.function()`)
- Gradual typing with optional type annotations
- LSP server implementation
- Code formatter
- REPL (Read-Eval-Print Loop)

### v0.3 (Planned)
- Async/await concurrency model
- Mark-and-Sweep garbage collection
- Performance optimizations
- Enhanced error recovery

### v1.0+ (Planned)
- JIT compilation
- Static type inference
- Package registry
- Debugger protocol
- Production-ready VM as default

---

[Unreleased]: https://github.com/ismobaga/izilang/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/ismobaga/izilang/releases/tag/v0.1.0

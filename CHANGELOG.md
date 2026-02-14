# Changelog

All notable changes to IziLang will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added - Runtime & VM Hardening
- **Stack Overflow Protection**: Automatic detection and prevention of stack overflow from deep recursion (max depth: 256)
- **Memory Statistics Tracking**: New `--memory-stats` flag to display memory usage report
- **Benchmark Command**: New `izi bench` command for performance testing with customizable iterations
- **REPL Enhancements**:
  - `:vars` command to show all defined variables in current session
  - `:tasks` command (placeholder for future async support)
  - State persistence across REPL commands
- **Runtime Safety Limits**: Configurable call depth and stack size monitoring
- **Memory Profiling Infrastructure**: Framework for tracking allocations by type

### Documentation
- Added comprehensive [Runtime Hardening Guide](docs/RUNTIME_HARDENING.md)
- Updated CLI help with new commands and options
- Documented memory statistics API

### Technical Improvements
- Separated REPL execution from one-shot execution for better state management
- Added `getCallDepth()` and `getStackSize()` APIs for runtime introspection
- Improved error messages for stack overflow scenarios

## [0.3.0] - 2026-02-11 (In Progress)

### Added - Gradual Typing System
- **Optional Type Annotations**: Add type hints to variables and functions without breaking dynamic nature
- **Type Checking**: `izi check` command validates type-annotated code
- **Built-in Types**: Number, String, Bool, Nil, Array, Map, Any, Void
- **Complex Types**: Array<T>, Map<K,V>, Function(T1, T2) -> R
- **Type Compatibility**: Any type works with all other types

### Added - Classes and OOP
- **Class Declarations**: Define classes with fields and methods
- **Constructor Method**: Special `constructor()` method for initialization
- **This Binding**: `this` keyword refers to current instance in methods
- **Property Access**: `obj.field` and `obj.method()` syntax
- **Property Assignment**: `obj.field = value` syntax
- **Type Annotations in Classes**: Fields and methods support type hints

### Added - Semantic Analysis
- **Static Analysis**: Analyze code without executing via `izi check`
- **Unused Variable Warnings**: Detect variables that are defined but never used
- **Dead Code Detection**: Warn about code after return statements
- **Unreachable Code**: Identify statements that can never execute
- **Control Flow Validation**: Error on break/continue outside loops
- **Scope Validation**: Error on return outside functions

### Added - Garbage Collection
- **Mark-and-Sweep GC**: Replaced reference counting with proper GC
- **Cyclic Reference Support**: GC handles circular data structures
- **GC Tuning**: Environment variables for threshold and debugging
- **Memory Statistics**: Track allocations and collections

### Added - VM Improvements
- **Import System**: Bytecode VM now supports full module imports
- **Performance Baseline**: 3x faster than interpreter for most operations
- **Benchmark Command**: `izi bench` for performance testing
- **Semantic Parity**: VM behavior matches interpreter exactly

### Added - Enhanced Error Reporting
- **Complete Stack Traces**: Show full call stack with file/line/column
- **Source Code Context**: Display problematic code with visual indicators
- **Better Error Messages**: More descriptive and actionable errors
- **Position Tracking**: All AST nodes track source location

### Added - Documentation
- **v0.3 Specification**: Complete language spec for new features
- **Migration Guide**: Step-by-step guide from v0.2 to v0.3
- **Type System Guide**: How to use gradual typing
- **Class Tutorial**: OOP patterns and best practices

### Improved - Type System
- **AST Extension**: Added TypeAnnotation struct for type representation
- **Type Inference Hooks**: Prepared for future type inference (v0.4)
- **Type Compatibility**: Flexible type checking with Any type

### Improved - Parser (Future Work)
- Parse type annotations in variable and function declarations
- Support class syntax parsing
- Property access expression parsing

### Fixed - Known Issues
- VM import compilation now implemented
- Cyclic references no longer cause memory leaks
- Stack traces work in both interpreter and VM

### Documentation
- [V03_SPECIFICATION.md](docs/V03_SPECIFICATION.md)
- [MIGRATION_V03.md](docs/MIGRATION_V03.md)
- Updated README.md with v0.3 features

### Backward Compatibility
- ✅ **100% backward compatible** with v0.2
- All v0.2 code runs unchanged
- Type annotations are optional
- Classes are new syntax (no conflicts)

### Known Limitations (v0.3)
- Type inference not yet implemented (must annotate explicitly)
- No inheritance or interfaces for classes
- No visibility modifiers (public/private)
- Parser support for new syntax coming in next commit

## [0.2.0] - 2026-02-11

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

# IziLang Project Status

**Last Updated**: February 21, 2026  
**Version**: v0.4.0-dev (v0.3.0 released, v0.4.0 in progress)  
**Status**: Active Development

---

## Executive Summary

IziLang is a modern programming language with a **dual execution model** (tree-walking interpreter and bytecode VM) implemented in C++20. The project is actively developing **v0.4.0** after shipping v0.3.0. The codebase has **185 test cases with 1,087 assertions all passing**. Key v0.4.0 features already landed include the `izi fmt` code formatter and async/await concurrency primitives.

### Maturity Level: ⭐⭐⭐⭐ (4/5) - v0.4.0 In Progress
- **Core Language**: ✅ Mature, feature-complete including async/await, macros, OOP, gradual typing
- **Runtime**: ✅ Tree-walking interpreter stable; VM functional (experimental)
- **Tooling**: ✅ Full CLI suite (`run`, `repl`, `fmt`, `check`, `bench`, `test`, `compile`, `chunk`)
- **Ecosystem**: ⚠️ LSP server and package manager in development
- **Documentation**: ✅ Excellent, comprehensive, up-to-date
- **Design Decisions**: ✅ All critical decisions frozen

---

## 1. Codebase Structure

### Repository Organization
```
izilang/
├── src/                    # Core implementation (~2,700 LOC C++)
│   ├── ast/               # Abstract Syntax Tree (✅ Complete)
│   ├── common/            # Shared types & utilities (✅ Complete)
│   ├── parse/             # Lexer & Parser (✅ Complete)
│   ├── interp/            # Tree-walking interpreter (✅ Complete)
│   ├── compile/           # Bytecode compiler (✅ Complete)
│   ├── bytecode/          # Virtual Machine (⚠️ Has bugs)
│   └── main.cpp           # CLI entry point (✅ Complete)
├── tests/                 # Test suite (✅ 63 tests, 100% passing)
├── tools/                 # Tooling ecosystem (⚠️ Partial)
│   ├── lsp/              # LSP server (⚠️ Framework only)
│   ├── pkg/              # Package manager (⚠️ Spec only)
│   └── vscode-extension/ # VS Code support (⚠️ Basic syntax)
├── std/                   # Standard library modules (✅ 46 functions)
├── playground/            # Web playground (⚠️ UI only)
├── docs/                  # Documentation (✅ Excellent)
└── examples/              # Code examples (✅ Good coverage)
```

**Clarity Assessment**: ✅ **EXCELLENT**
- Clear separation of concerns
- Logical directory structure
- Well-documented components

---

## 2. Build System

### Technology: Premake5
**Status**: ✅ **STABLE**

### Supported Platforms
- ✅ Linux (primary target)
- ✅ macOS (tested)
- ✅ Windows (tested)

### Build Configurations
- **Debug**: Full symbols, assertions enabled
- **Release**: Optimizations enabled (`-O3`)

### Build Commands
```bash
./premake5 gmake2           # Generate makefiles
make config=debug           # Debug build
make config=release         # Release build
./bin/Debug/izi/izi file.iz # Run interpreter
./bin/Debug/tests/tests     # Run test suite
```

**Build Time**: ~2-3 seconds (clean build)  
**Binary Size**: ~1.2 MB (debug), ~400 KB (release)

**Assessment**: ✅ Build system is **stable, fast, and well-configured**.

---

## 3. CI/CD Status

### GitHub Actions Workflows

| Workflow | Status | Purpose |
|----------|--------|---------|
| **ci.yml** | ✅ Active | Multi-platform build & test |
| **code-quality.yml** | ✅ Active | clang-format, clang-tidy, cppcheck |
| **release.yml** | ✅ Active | Automated binary releases |
| **deploy-playground.yml** | ✅ Active | GitHub Pages deployment |

### CI Coverage
- ✅ Builds on Ubuntu, macOS, Windows
- ✅ Runs full test suite (63 tests)
- ✅ Code quality checks enforced
- ✅ Automated release artifacts

**Assessment**: ✅ **PRODUCTION-GRADE** CI/CD pipeline

---

## 4. Test Coverage

### Test Framework: Catch2 v2.13.10
**Status**: ✅ **COMPREHENSIVE**

### Test Statistics
- **Total Test Cases**: 185
- **Total Assertions**: 1,087
- **Pass Rate**: 100% ✅
- **Execution Time**: < 2 seconds

### Test Categories

| Category | Test Cases | Coverage |
|----------|------------|----------|
| **Lexer** | 7 | Tokenization, operators, keywords |
| **Values** | 5 | Type system, truthiness |
| **Integration** | 9 | End-to-end language features |
| **Modules** | 6 | Import/export system |
| **Pattern Matching** | 7 | Match expressions, guards |
| **Collections** | 8 | Arrays, maps, operations |
| **First-Class Functions** | 15 | Closures, higher-order functions |
| **Try/Catch** | 13 | Exception handling |
| **VM Simple** | Various | Bytecode execution |
| **OOP / Classes** | Various | Classes, inheritance, `this` |
| **Async/Await** | 6 | Async functions, await expressions |
| **Macros** | 5 | Macro definitions and expansion |
| **Ternary Operator** | 9 | `condition ? a : b` syntax |
| **Formatter** | Various | `izi fmt` code formatting |
| **Semantic Analysis** | Various | Static checks, unused vars |
| **HTTP / Net** | Various | Native networking modules |

### Test Files
```
tests/
├── test_lexer.cpp                 # Tokenization tests
├── test_value.cpp                 # Value type tests
├── test_integration.cpp           # End-to-end tests
├── test_modules.cpp               # Module system tests
├── test_pattern_matching.cpp      # Pattern matching tests
├── test_collections.cpp           # Array/map tests
├── test_first_class_functions.cpp # Function tests
├── test_try_catch.cpp             # Exception handling tests
├── test_vm_simple.cpp             # VM bytecode tests
├── test_vm_stack.cpp              # VM stack operations
├── test_classes.cpp               # OOP and inheritance
├── test_async.cpp                 # Async/await concurrency
├── test_macros.cpp                # Macro system
├── test_ternary_operator.cpp      # Ternary operator
├── test_formatter.cpp             # Code formatter
├── test_semantic_analysis.cpp     # Semantic analysis
├── test_optimizer.cpp             # AST optimizer
├── test_http.cpp                  # HTTP native module
├── test_net.cpp                   # Net native module
├── test_ipc.cpp                   # IPC module
├── test_error_system.cpp          # Error system
└── test_parser.cpp                # Parser tests
```

**Assessment**: ✅ **GOOD** coverage for core features. Missing: error recovery, edge cases, performance tests.

---

## 5. Language Core

### 5.1 Lexer
**Status**: ✅ **COMPLETE**

#### Implemented Features
- ✅ All token types (operators, keywords, literals)
- ✅ Line and column tracking
- ✅ String literals with escape sequences
- ✅ String interpolation with `${}` syntax
- ✅ Number literals (integers, decimals)
- ✅ Identifiers and keywords
- ✅ Error reporting with source locations
- ✅ Single-line comments (`//`)
- ✅ Multi-line block comments (`/* */`)

#### Comment Support
- ✅ **Single-line comments**: `// comment` - Working correctly
- ✅ **Block comments**: `/* comment */` - Working correctly with nesting detection
- ✅ **Unterminated comment detection**: Proper error messages

**Correctness**: ✅ **100%** - All features working as designed

---

### 5.2 Parser
**Status**: ✅ **ROBUST**

#### Parsing Techniques
- ✅ Recursive descent with precedence climbing
- ✅ Comprehensive error messages
- ⚠️ Limited error recovery (stops on first error)

#### Operator Precedence (Lowest to Highest)
```
1. or              (logical OR)
2. and             (logical AND)
3. ==, !=          (equality)
4. <, >, <=, >=    (comparison)
5. +, -            (addition, subtraction)
6. *, /            (multiplication, division)
7. !, -            (unary NOT, negation)
8. [], .           (indexing, member access)
```

**Robustness**: ✅ **GOOD**
- Error recovery: ⚠️ Basic (synchronizes to statement boundaries)
- Error messages: ✅ Clear with line/column info

---

### 5.3 AST Design
**Status**: ✅ **EXCELLENT**

#### Design Pattern: Visitor Pattern
- All expression nodes inherit from `Expr` base class
- All statement nodes inherit from `Stmt` base class
- Double-dispatch via `accept(Visitor&)` methods
- Separate visitors for expressions and statements

#### Expression Types (12 types)
```cpp
LiteralExpr         // nil, true, 42, "string"
VariableExpr        // identifier
BinaryExpr          // left op right
UnaryExpr           // op right
GroupingExpr        // (expression)
AssignExpr          // identifier = expression
CallExpr            // callee(args...)
IndexExpr           // array[index]
ArrayExpr           // [elem1, elem2, ...]
MapExpr             // {key1: val1, key2: val2}
MatchExpr           // match value { patterns }
InterpolationExpr   // f"Hello {name}"
```

#### Statement Types (11 types)
```cpp
ExprStmt            // expression;
BlockStmt           // { statements... }
VarStmt             // var name = value;
IfStmt              // if (condition) { ... } else { ... }
WhileStmt           // while (condition) { ... }
ForStmt             // for (init; cond; incr) { ... }
FunctionStmt        // fn name(params) { body }
ReturnStmt          // return expression;
BreakStmt           // break;
ContinueStmt        // continue;
ImportStmt          // import { names } from "module";
ExportStmt          // export var/fn name...
MatchStmt           // match value { cases }
TryStmt             // try { } catch (e) { } finally { }
```

**Extensibility**: ✅ **EXCELLENT** - Easy to add new node types

---

### 5.4 Semantic Analysis
**Status**: ⚠️ **MINIMAL**

#### Implemented
- ✅ Symbol resolution (variables, functions)
- ✅ Scope management (nested blocks)
- ✅ Basic error detection (undefined variables)

#### NOT Implemented
- ❌ Type checking (all dynamic at runtime)
- ❌ Dead code detection
- ❌ Unused variable warnings
- ❌ Flow analysis (definite assignment, reachability)
- ❌ Constant folding
- ❌ Loop invariant detection

**Maturity**: ⚠️ **EXPERIMENTAL** - Needs significant work for production use

---

## 6. Runtime

### 6.1 Execution Models

IziLang supports **two execution modes**:

#### Tree-Walking Interpreter
**Status**: ✅ **COMPLETE & STABLE**
- Direct AST traversal
- Slower but easier to debug
- **Currently the default** (`useVM = false`)
- Uses C++ exceptions for control flow

#### Bytecode Virtual Machine
**Status**: ⚠️ **IMPLEMENTED BUT BUGGY**
- Stack-based VM with 30+ opcodes
- Faster execution (theoretical)
- **Has pre-existing bugs** preventing reliable use
- Full instruction set implemented

**Default Mode**: Tree-walking interpreter (VM disabled due to bugs)

---

### 6.2 Error Handling
**Status**: ✅ **IMPLEMENTED**

#### Interpreter Mode
- Uses C++ exceptions (`RuntimeError`, `BreakSignal`, `ContinueSignal`, `ReturnValue`)
- Stack unwinding for control flow

#### VM Mode
- Exception handler stack (`tryStack`)
- Opcodes: `OP_TRY`, `OP_CATCH`, `OP_FINALLY`, `OP_END_TRY`
- Try/catch/finally blocks fully supported

#### Error Reporting
```
Runtime Error at line 3, column 18:
  3 | var result = name + age;
    |                  ^
Cannot add string and number. Operands must be two numbers or two strings.
```

**Features**:
- ✅ Line and column tracking
- ✅ Code snippets with carets
- ✅ Color-coded output
- ✅ Descriptive messages

**Implementation Quality**: ✅ **GOOD**

---

### 6.3 Stack Trace Support
**Status**: ⚠️ **PARTIAL**

#### Current Implementation
- ✅ Captures call stack in exceptions
- ✅ Shows function names
- ⚠️ Line numbers sometimes missing
- ⚠️ No source file info in traces

#### Example Output
```
RuntimeError: Division by zero
  at calculate()
  at main()
```

**Improvement Needed**: Full source location tracking in all stack frames

---

### 6.4 Garbage Collection Strategy
**Status**: ✅ **DECIDED** (for v0.1)

#### Current Memory Management (v0.1)
- **Strategy**: Reference counting via `std::shared_ptr`
- `std::shared_ptr` for runtime objects (functions, arrays, maps)
- `std::unique_ptr` for AST nodes
- **Status**: Production-ready for v0.1

#### Decision for v0.1
**Chosen**: Reference counting (Option 1)

**Rationale**:
- Already implemented and stable
- Predictable, deterministic deallocation
- Low latency (no garbage collection pauses)
- Sufficient for v0.1 use cases
- Simple mental model for developers

#### Known Limitations
- ⚠️ Cannot handle circular references automatically
- ⚠️ Reference counting overhead on assignments
- ⚠️ No control over collection timing

#### User Guidance
- Avoid creating circular references in data structures
- Use weak references if circular structures needed (future feature)
- For v0.1: Design data structures to avoid cycles

#### Future Evolution
- **v0.2**: Add cycle detection warnings and diagnostics
- **v0.3**: Evaluate Mark-and-Sweep GC with generational extension
- **v1.0+**: Consider hybrid approach (RC + cycle detection like Swift)

**This decision is frozen for v0.1 and documented in docs/DECISIONS.md**

---

### 6.5 Concurrency Model
**Status**: ❌ **UNDEFINED**

#### Current State
- ✅ Async functions: `async fn fetch(url) { ... }`
- ✅ `await` keyword for calling async functions
- ✅ `ASYNC` and `AWAIT` tokens in lexer
- ✅ `AsyncFunctionStmt` and `AwaitExpr` in AST
- ⚠️ Event loop (cooperative scheduling) — runtime hooks present, full I/O integration in progress

#### Future Enhancement
- Full non-blocking I/O via `std.async` module
- Promise chaining: `then()`, `catch()`, `finally()`

**Decision**: ✅ **Async/Await with cooperative event loop**

---

## 7. Tooling

### 7.1 CLI Commands
**Status**: ✅ **FULL-FEATURED**

#### Available Commands
```bash
izi                    # Start interactive REPL (default)
izi run <file>         # Execute a source file
izi build <file>       # Compile/check without executing
izi check <file>       # Parse and analyze (semantic checks)
izi compile <file>     # Compile to native executable
izi chunk <file>       # Compile to bytecode chunk (.izb)
izi test [pattern]     # Run test files (searches *.iz in tests/)
izi repl               # Start interactive REPL
izi bench <file>       # Run performance benchmark
izi fmt <file>         # Format source code
izi version            # Show version information
izi help [command]     # Show help for a specific command

# Global flags
--vm                   # Use bytecode VM (experimental)
--interp               # Use tree-walker interpreter (default)
--debug                # Enable debug/verbose output
--optimize, -O         # Enable optimizations (default: on)
--no-optimize, -O0     # Disable optimizations
--memory-stats         # Show memory usage statistics
```

**Stability**: ✅ Full CLI suite implemented and stable

---

### 7.2 Diagnostics Quality
**Status**: ✅ **GOOD**

#### Features
- ✅ Spans (line, column tracking)
- ✅ Hints (caret pointing to error)
- ✅ Descriptive messages
- ✅ Color-coded output

#### Example
```
Parse Error at line 5, column 12:
  5 |     if x == {
    |            ^
Expected expression, found '{'.
```

**Quality**: ✅ **PRODUCTION-READY**

---

### 7.3 Code Formatter
**Status**: ✅ **IMPLEMENTED**

#### Features
- ✅ `izi fmt <file>` command
- ✅ `izi fmt --check <file>` (check-only mode)
- ✅ Auto-indent and consistent spacing
- ✅ Operator spacing rules
- ✅ Brace formatting

**Priority**: ✅ Complete (landed in v0.4.0)

---

### 7.4 LSP / Editor Support
**Status**: ⚠️ **FRAMEWORK ONLY**

#### Language Server Protocol (LSP)
- ✅ Server framework exists (`tools/lsp/`)
- ❌ No diagnostics
- ❌ No autocompletion
- ❌ No hover information
- ❌ No go-to-definition

#### VS Code Extension
- ✅ Syntax highlighting (basic)
- ✅ Extension manifest
- ❌ LSP client not connected
- ❌ No debugging support

**Status**: ⚠️ **SKELETON ONLY** - Needs full implementation

---

## 8. Standard Library

### 8.1 Module Structure
**Status**: ✅ **DEFINED**

#### Available Modules
```
std.math       # Mathematical functions
std.string     # String manipulation
std.array      # Array operations
std.io         # File I/O
```

### 8.2 Core Modules

#### std.math (11 functions + 2 constants)
```
Functions: sqrt, pow, sin, cos, tan, floor, ceil, round, abs, min, max
Constants: PI (3.14159265359), E (2.71828182846)
```

#### std.string (10 functions)
```
substring, split, toUpper, toLower, trim, replace,
indexOf, startsWith, endsWith, join
```

#### std.array (7 functions)
```
map, filter, reduce, sort, reverse, concat, slice
```

#### std.io (4 functions)
```
readFile, writeFile, appendFile, fileExists
```

### 8.3 Global Functions
```
print(value)    # Output to stdout
len(collection) # Get length of array/map/string
```

**Total Native Functions**: 46

### 8.4 Import Resolution
**Status**: ✅ **COMPLETE**

#### Import Syntax
```izi
// Named imports
import { sqrt, pow } from "math";

// Wildcard imports with namespace object
import * as math from "math";
// Access: math["sqrt"](16) or iterate: for (key in math) { ... }

// Simple module import
import "mymodule.iz";
```

#### Implementation Status
- ✅ **Named imports**: Import specific functions by name
- ✅ **Wildcard imports**: Create namespace object (Map) with all exports
- ✅ **Module objects**: Fully functional with bracket notation access
- ✅ **Circular import handling**: Lazy loading prevents infinite loops
- ✅ **Native module resolution**: Built-in modules (math, string, array, io)
- ✅ **File-based modules**: Relative paths and module search

**Resolution Rules**: 
1. Check for native modules first (math, string, array, io)
2. Resolve relative paths (./module.iz, ../module.iz)
3. Add .iz extension automatically if not present
4. Cache imported modules to prevent re-execution

**Namespace Object Details**:
- Wildcard imports create a Map containing all exported values
- Access via bracket notation: `module["functionName"](args)`
- Can enumerate keys: `for (key in module) { ... }`
- Future enhancement (v0.2): Add dot notation `module.functionName()`

---

## 9. Maturity Gap Analysis

### Status Classification

| Component | Status | Notes |
|-----------|--------|-------|
| **Lexer** | ✅ Stable | Production-ready, all tokens including `async`, `await`, `macro` |
| **Parser** | ✅ Stable | Full language grammar, including async/await, macros, ternary |
| **AST** | ✅ Stable | 14 expression types, 15+ statement types |
| **Interpreter** | ✅ Stable | Default execution mode, all features supported |
| **VM** | ⚠️ Experimental | Functional, opt-in (`--vm` flag) |
| **Compiler** | ✅ Stable | Bytecode generation for all language constructs |
| **Optimizer** | ✅ Stable | Constant folding, dead code elimination |
| **Error Handling** | ✅ Stable | Try/catch/finally, stack traces |
| **Stack Traces** | ⚠️ Partial | Function names shown; file names sometimes missing |
| **GC Strategy** | ✅ Decided | Reference counting (std::shared_ptr) |
| **Module System** | ✅ Stable | Named and wildcard imports, dot-notation access |
| **Async/Await** | ✅ Implemented | `async fn`, `await` expressions |
| **Macros** | ✅ Implemented | `macro name(params) { body }`, expansion via `name!()` |
| **OOP / Classes** | ✅ Stable | Classes, inheritance, constructors, `this` |
| **Gradual Typing** | ✅ Stable | Optional type annotations on vars and functions |
| **Pattern Matching** | ✅ Stable | Match expressions with guards |
| **Ternary Operator** | ✅ Stable | `condition ? a : b` |
| **CLI** | ✅ Stable | Full suite: run/repl/fmt/check/test/bench/compile |
| **Formatter** | ✅ Stable | `izi fmt` with check mode |
| **Semantic Analysis** | ✅ Stable | `izi check` — unused vars, dead code, type hints |
| **Diagnostics** | ✅ Stable | Color-coded, source-location errors |
| **Stdlib** | ✅ Stable | 46+ functions: math, string, array, io, json, time, regex, http, net, log, ipc |
| **LSP** | ⚠️ Framework | Server skeleton; diagnostics/completion not yet wired |
| **Package Manager** | ⚠️ Spec | Design documented; implementation pending |
| **Tests** | ✅ Stable | 185 test cases, 1,087 assertions, 100% passing |
| **CI/CD** | ✅ Stable | Multi-platform (Linux, macOS, Windows) |
| **Documentation** | ✅ Stable | 20+ markdown documents |

### Technical Debt

1. **VM completeness** — Some edge cases in class/async support under VM mode
2. **Stack traces** — Source file names missing from some frames
3. **LSP** — Framework exists but not connected to real diagnostics
4. **Package Manager** — Backend not yet implemented
5. **Async I/O** — Async syntax is parsed and interpreted; full event loop I/O integration pending

---

## 10. Design Decisions Required

### 10.1 Primary Execution Model
**Status**: ✅ **DECIDED** (for v0.1)

#### Current State
- Tree-walking interpreter: ✅ Working, default, production-ready
- Bytecode VM: ✅ Implemented, experimental, opt-in

#### Decision
**Chosen**: **Hybrid** - Interpreter for dev and production, VM for experimentation

**Implementation**:
- Default: Tree-walking interpreter (`izi run script.iz`)
- Optional: Bytecode VM (`izi run --vm script.iz`)
- Both modes fully functional
- Interpreter recommended for v0.4 and beyond

**Status**: Frozen and documented in docs/DECISIONS.md

---

### 10.2 Type Inference Boundaries
**Status**: ✅ **DECIDED**

#### Current State
- Gradual typing implemented: optional type annotations on variables and function parameters
- No static type checking yet (annotations are runtime-checked only)
- Type inference (infer types from assignment) planned for v1.0

#### Decision
**Chosen**: Gradual typing (optional annotations) with runtime checks. Full static inference deferred to post-v1.0.

---

### 10.3 Error Handling Contract
**Status**: ✅ **DECIDED**

#### Decision
- Use try/catch/finally exceptions (not Result types)
- Stack traces included in exceptions
- Errors are first-class values

**Rationale**: Familiar to users from JS/Python/Java

---

### 10.4 Concurrency Model
**Status**: ✅ **DECIDED**

#### Decision
**Chosen**: Async/Await with cooperative scheduling

- `async fn` declares asynchronous functions
- `await` suspends the current function until the awaited call resolves
- Full non-blocking I/O via `std.async` (in progress for v0.4)

---

### 10.5 Stdlib Philosophy
**Status**: ✅ **DECIDED**

#### Current Approach
- **Batteries-included** — Rich built-in modules
- Named imports add functions to global scope
- Wildcard imports create namespace objects with dot-notation access

**Import Options**:
```izi
// Named imports (global scope)
import { sqrt, pow } from "math";
sqrt(16);  // Direct call

// Wildcard import (namespaced)
import * as math from "math";
math.sqrt(16);  // Dot-notation access
```

**Status**: Frozen and documented in docs/DECISIONS.md

---

### 10.6 Backward Compatibility Policy
**Status**: ✅ **DECIDED**

#### Policy
- Breaking changes **permitted** in minor versions (v0.x)
- All breaking changes documented in CHANGELOG.md
- Migration guides provided for major changes
- Full SemVer compliance starting at v1.0

**Status**: Frozen and documented in docs/DECISIONS.md

---

## 11. Overall Assessment

### Strengths ✅
1. **Solid architecture** - Clean separation, excellent design patterns
2. **Comprehensive features** - Most language features implemented and tested
3. **Excellent documentation** - 20+ markdown files, well-written, up-to-date
4. **Robust build system** - Multi-platform (Linux, macOS, Windows), fast builds
5. **Production CI/CD** - Automated testing, releases, deployment
6. **Strong test coverage** - 185 tests, 1,087 assertions, 100% passing
7. **Rich stdlib** - math, string, array, io, json, time, regex, http, net, log, ipc
8. **Complete module system** - Named and wildcard imports with dot-notation
9. **Clear error messages** - Rust-quality diagnostics with source locations
10. **Full CLI toolchain** - run, repl, fmt, check, test, bench, compile

### Open Items ⚠️
1. **VM completeness** - Some edge cases in VM mode for classes/async
2. **Stack traces** - Source file names missing from some frames
3. **LSP server** - Framework exists, diagnostics/completion not yet wired
4. **Package manager** - Design spec done, backend implementation pending
5. **Async I/O** - Syntax ready, full event loop I/O integration pending
6. **Debugger (DAP)** - Not yet started

---

## 12. Recommendation

### Current State: **v0.4.0 In Progress** 🟡

IziLang has achieved a strong v0.3.0 foundation and is actively building v0.4.0.

### Completed in v0.4.0 so far
1. ✅ Code formatter (`izi fmt`)
2. ✅ Async/await syntax and runtime
3. ✅ Macro system

### v0.4.0 Remaining
1. Complete LSP server (diagnostics + autocompletion)
2. Full async I/O integration (`std.async`)
3. Package manager backend

### Path to v1.0 (Q4 2026)
1. Stabilize LSP server
2. Ship package manager MVP
3. Complete debugger (DAP)
4. Language freeze
5. Ecosystem growth (50+ packages)

---

## 13. Conclusion

**IziLang v0.3.0 is released and v0.4.0 is in active development.**

**Status**: 🟡 **v0.4.0 In Progress** - Strong foundation, tooling being completed

**Key Achievements**:
- ✅ 185 tests passing (1,087 assertions, 100% success rate)
- ✅ Full CLI toolchain (run, repl, fmt, check, test, bench, compile)
- ✅ Classes and OOP with full inheritance
- ✅ Gradual typing system (optional annotations)
- ✅ Mark-and-Sweep GC
- ✅ Async/await syntax and runtime
- ✅ Macro system
- ✅ Code formatter (`izi fmt`)
- ✅ Rich stdlib (math, string, array, io, json, time, regex, http, net, log, ipc)
- ✅ All design decisions frozen and documented

**Next Steps**:
1. Complete LSP server wiring (diagnostics + autocompletion)
2. Ship async I/O integration (`std.async`)
3. Package manager backend
4. Tag v0.4.0 release

**Trajectory**: Positive — strong foundation with active feature development

---

**Document maintained by**: IziLang Development Team  
**Next review**: After v0.4.0 release

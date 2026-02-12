# IziLang v1.x Roadmap

**Last Updated**: February 2026  
**Current Version**: v0.2.0  
**Target**: v1.0.0 by Q4 2026

---

## Executive Summary

This document outlines the path from the current state (v0.2) to a production-ready v1.0 release, and beyond to v1.x and v2.0. It builds upon the [Language Manifesto](LANGUAGE_MANIFESTO.md) and [Feature Inventory](FEATURE_INVENTORY.md) to provide a concrete, time-bound implementation plan.

**Key Milestones**:
- **v0.3** (Q2 2026) — Language Power: OOP, typing, semantic analysis
- **v0.4** (Q3 2026) — Developer Experience: Tooling ecosystem
- **v1.0** (Q4 2026) — Stable: Production-ready with backward compatibility
- **v1.x** (2027+) — Maturity: Performance, advanced features
- **v2.0** (2028+) — Evolution: Breaking improvements

---

## Table of Contents

1. [v1.0 Scope Definition](#v10-scope-definition)
2. [v0.3 — Language Power](#v03--language-power-q2-2026)
3. [v0.4 — Developer Experience](#v04--developer-experience-q3-2026)
4. [v1.0 — Stable](#v10--stable-q4-2026)
5. [v1.1 — Productivity](#v11--productivity-2027-q1)
6. [v1.2 — Ecosystem](#v12--ecosystem-2027-q2)
7. [v2.0 — Advanced](#v20--advanced-2028)
8. [Risk Management](#risk-management)
9. [Success Metrics](#success-metrics)

---

## v1.0 Scope Definition (MVP)

### Must-Have Features

#### 1. Core Language (Stable)
- ✅ Variables, functions, control flow
- ✅ First-class functions and closures
- ✅ Pattern matching
- ✅ Exception handling (try/catch/finally)
- ✅ Module system (import/export)
- ✅ String interpolation
- ✅ Arrays and maps

#### 2. Object-Oriented Programming
- 📋 Classes with fields and methods
- 📋 Constructors
- 📋 `this` binding
- 📋 Property access (`obj.field`, `obj.method()`)
- ❌ Inheritance (deferred to v1.1)
- ❌ Interfaces/traits (deferred to v1.2)

#### 3. Type System
- 📋 Optional type annotations
- 📋 Runtime type checking
- 📋 Basic type inference (v0.4)
- ❌ Full static analysis (deferred to v1.1)

#### 4. Standard Library
- ✅ std.math (13 functions + constants)
- ✅ std.string (10 functions)
- ✅ std.array (7 functions)
- ✅ std.io (4 functions)
- 📋 std.json (parse, stringify)
- 📋 std.regex (match, replace)
- 📋 std.time (now, format, parse)
- 📋 std.env (get, set, list)
- 📋 std.process (exec, spawn)

**Target**: 100+ functions by v1.0

#### 5. Error Handling
- ✅ Try/catch/finally blocks
- ✅ Exception propagation
- ✅ Stack traces
- 📋 Better error messages with suggestions
- 📋 Error codes for documentation

#### 6. Execution Model
- ✅ Tree-walking interpreter (stable, default)
- 📋 Bytecode VM (fix bugs, make production-ready)
- 📋 Deterministic compilation
- 📋 Reproducible builds

#### 7. Development Tools
- 📋 REPL (interactive shell)
- 📋 Code formatter (`izi fmt`)
- 📋 LSP server (diagnostics, completion, hover)
- 📋 Semantic analyzer (`izi check`)
- 📋 Test framework (built-in)

#### 8. Build System
- ✅ Cross-platform (Linux, macOS, Windows)
- ✅ Fast builds (< 5 seconds)
- ✅ CI/CD pipeline
- ✅ Multi-configuration (debug, release)

#### 9. Documentation
- ✅ Language specification
- ✅ Standard library reference
- 📋 Tutorial (getting started)
- 📋 Cookbook (common patterns)
- 📋 API documentation (for embedding)

#### 10. Backward Compatibility
- 📋 Semantic versioning adopted
- 📋 Deprecation policy defined
- 📋 Migration guides for breaking changes
- 📋 v1.0+ guarantees no breaking changes in minor versions

### Explicitly Excluded from v1.0

These features are **NOT** in v1.0 scope:

- ❌ Async/await concurrency (v1.1+)
- ❌ JIT compilation (v1.2+)
- ❌ Reflection API (v1.2+)
- ❌ Macros / metaprogramming (v2.0+)
- ❌ Multiple inheritance (never)
- ❌ Goto statements (never)
- ❌ Implicit type coercion (never)

### Completion Criteria

v1.0 can be released when:

1. ✅ All must-have features implemented
2. ✅ Test coverage > 90%
3. ✅ Documentation 100% complete
4. ✅ Zero known critical bugs
5. ✅ Performance within 2x of Python 3.11
6. ✅ Semantic versioning policy adopted
7. ✅ At least 10 example programs
8. ✅ Multi-platform binaries available
9. ✅ Public registry operational (basic)
10. ✅ Community guidelines established

---

## v0.3 — Language Power (Q2 2026)

**Target Date**: March - May 2026 (3 months)  
**Focus**: Complete core language features  
**Status**: 35% complete

### Objectives

1. Add classes and object-oriented programming
2. Implement gradual typing system
3. Stabilize bytecode VM
4. Enhance standard library
5. Improve error messages

### Features

#### 1. Classes and OOP
**Timeline**: 4 weeks  
**Owner**: Core team

**Tasks**:
- ✅ AST nodes created (ClassStmt, PropertyExpr, SetPropertyExpr, ThisExpr)
- Week 1: Parser support for class syntax
- Week 2: Interpreter implementation (instantiation, methods, `this`)
- Week 3: VM bytecode support for classes
- Week 4: Testing and documentation

**Examples**:
```izi
class Person {
    var name: string;
    var age: number;
    
    fn constructor(name, age) {
        this.name = name;
        this.age = age;
    }
    
    fn greet() {
        print(f"Hello, I'm {this.name}!");
    }
    
    fn birthday() {
        this.age = this.age + 1;
    }
}

var alice = Person("Alice", 30);
alice.greet();
alice.birthday();
print(alice.age);  // 31
```

**Deliverables**:
- Class declaration and instantiation
- Instance methods with `this` binding
- Property access and assignment
- Constructor methods
- 20+ test cases
- Documentation chapter

**Deferred to v0.4**:
- Inheritance (`class Child extends Parent`)
- Static methods
- Private fields
- Getters/setters

#### 2. Gradual Typing System
**Timeline**: 4 weeks  
**Owner**: Core team

**Tasks**:
- ✅ Type system design complete
- Week 1: Parser support for type annotations
- Week 2: Runtime type checking
- Week 3: Type compatibility rules
- Week 4: Error messages and documentation

**Examples**:
```izi
// Type annotations in variables
var x: number = 42;
var name: string = "Alice";
var items: array<number> = [1, 2, 3];
var data: map<string, any> = { key: "value" };

// Type annotations in functions
fn add(a: number, b: number): number {
    return a + b;
}

// Type checking at runtime
var result = add(5, 3);      // OK
var error = add("5", 3);     // Runtime error: Expected number, got string
```

**Deliverables**:
- Type annotation syntax
- Runtime type checking
- Type error messages
- 30+ test cases
- Type system documentation

**Deferred to v0.4**:
- Type inference
- Generic types
- Union types
- Advanced type features

#### 3. Bytecode VM Stabilization
**Timeline**: 3 weeks  
**Owner**: Core team

**Tasks**:
- Week 1: Identify and document all VM bugs
- Week 2: Fix critical bugs (imports, exception handling)
- Week 3: Performance testing and optimization

**Goals**:
- ✅ VM passes all test cases
- ✅ Import system working in VM
- ✅ Exception handling in VM
- ✅ Performance within 3x of interpreter
- ✅ Stress testing (long-running programs)

**Success Criteria**:
- All 68+ tests pass in VM mode
- No crashes in 24-hour stress test
- Memory usage stable
- Ready to recommend for production

#### 4. Semantic Analysis
**Timeline**: 2 weeks  
**Owner**: Core team

**Tasks**:
- ✅ Core analyzer implemented
- Week 1: Integrate with CLI (`izi check`)
- Week 2: Add more checks (constant folding, flow analysis)

**Features**:
```bash
$ izi check src/

src/main.iz:15:9: warning: unused variable 'temp'
15 |     var temp = calculate();
   |         ^^^^

src/utils.iz:42:5: warning: unreachable code after return
42 |     return result;
43 |     print("done");  // <-- this line
   |     ^^^^^

2 warnings found
```

**Checks**:
- ✅ Unused variables
- ✅ Unreachable code
- ✅ Break/continue outside loops
- ✅ Return outside functions
- 📋 Uninitialized variables
- 📋 Type mismatches (with annotations)
- 📋 Dead code elimination opportunities

#### 5. Standard Library Expansion
**Timeline**: 2 weeks  
**Owner**: Community + core team

**New Modules**:

**std.json** (4 functions):
```izi
import { parse, stringify, parseFile, stringifyFile } from "json";

var data = parse('{"name": "Alice", "age": 30}');
var json = stringify(data);
```

**std.regex** (4 functions):
```izi
import { match, replace, split, test } from "regex";

var matched = match("hello", "h.*o");  // true
var replaced = replace("hello world", "world", "IziLang");
```

**std.time** (6 functions):
```izi
import { now, format, parse, sleep, timestamp, elapsed } from "time";

var current = now();
var formatted = format(current, "%Y-%m-%d %H:%M:%S");
sleep(1000);  // Sleep 1 second
```

**std.env** (4 functions):
```izi
import { get, set, list, has } from "env";

var home = env.get("HOME");
env.set("MY_VAR", "value");
var allVars = env.list();
```

**std.process** (4 functions):
```izi
import { exec, spawn, exit, args } from "process";

var result = exec("ls", ["-la"]);
print(result.stdout);

var child = spawn("python", ["script.py"]);
```

**Total New Functions**: 22 (bringing total to 68+)

#### 6. Enhanced Error Messages
**Timeline**: Ongoing

**Improvements**:
- Source file names in errors
- Line numbers in all stack traces
- Suggestion system (common fixes)
- Color-coded output
- Error code system (E0001, E0002, etc.)

**Example**:
```
Error E0308: Type mismatch at main.iz:12:18
  12 | var result = name + age;
     |                  ^ ^^^
     |                  | |
     |                  | expected string, found number
     |                  string

help: Convert `age` to string:
    var result = name + string(age);

note: In IziLang, types must match for addition.
      Use explicit conversion to avoid this error.

docs: https://izilang.org/errors/E0308
```

### Testing

- 📋 Add 100+ new test cases
- 📋 Achieve 90%+ test coverage
- 📋 Integration tests for new features
- 📋 Performance benchmarks

### Documentation

- 📋 v0.3 specification document
- 📋 Migration guide from v0.2
- 📋 Type system guide
- 📋 OOP tutorial
- 📋 Updated API reference

### Exit Criteria

v0.3 is complete when:

1. ✅ Classes fully implemented and tested
2. ✅ Gradual typing works with clear errors
3. ✅ VM is stable and production-ready
4. ✅ Standard library has 68+ functions
5. ✅ `izi check` command working
6. ✅ All tests pass (100% success rate)
7. ✅ Documentation complete
8. ✅ No blocking bugs

**Expected Release**: May 31, 2026

---

## v0.4 — Developer Experience (Q3 2026)

**Target Date**: June - August 2026 (3 months)  
**Focus**: Tooling and productivity  
**Status**: Not started

### Objectives

1. Build comprehensive tooling ecosystem
2. Enhance developer productivity
3. Improve debugging experience
4. Create learning resources
5. Start community building

### Features

#### 1. REPL (Interactive Shell)
**Timeline**: 2 weeks

**Features**:
```bash
$ izi repl
IziLang 0.4.0 REPL
Type :help for help, :quit to exit

> var x = 42;
> print(x + 8);
50

> fn factorial(n) {
...   if (n <= 1) return 1;
...   return n * factorial(n - 1);
... }
> print(factorial(5));
120

> :vars
x: 42
factorial: <function>

> :quit
Goodbye!
```

**Special Commands**:
- `:help` — Show help
- `:quit` — Exit REPL
- `:clear` — Clear screen
- `:vars` — Show all variables
- `:load <file>` — Load and execute file
- `:save <file>` — Save session to file

**Deliverables**:
- Multi-line input support
- History navigation (up/down arrows)
- Tab completion (variables, functions)
- Syntax highlighting
- Error recovery (don't exit on error)

#### 2. Code Formatter
**Timeline**: 2 weeks

**Features**:
```bash
izi fmt file.iz              # Format file in-place
izi fmt --check file.iz      # Check without modifying
izi fmt --diff file.iz       # Show changes
izi fmt src/                 # Format directory recursively
```

**Style Defaults**:
- 4-space indentation
- Spaces around binary operators
- No trailing whitespace
- Consistent braces placement
- Max line length: 100 characters

**Configuration** (`.izifmt.toml`):
```toml
[format]
indent = 4
max_line_length = 100
trailing_comma = true
space_around_operators = true
```

**Deliverables**:
- Format all syntax forms
- Preserve comments
- Configurable style
- CI integration

#### 3. Language Server Protocol (LSP)
**Timeline**: 6 weeks (largest feature)

**Phase 1: Core Features** (3 weeks):
- Diagnostics (syntax errors, warnings)
- Document symbols (functions, classes, variables)
- Hover information (basic)
- Go-to-definition (same file)

**Phase 2: Advanced Features** (3 weeks):
- Auto-completion (context-aware)
- Find references
- Rename symbol
- Code actions (quick fixes)
- Multi-file support

**Example Usage** (VS Code):
```
- Red underline: Syntax errors
- Yellow underline: Warnings
- Ctrl+Space: Auto-complete
- F12: Go to definition
- Shift+F12: Find all references
- F2: Rename symbol
```

**Deliverables**:
- LSP server binary
- VS Code extension update
- Vim/Neovim plugin
- Documentation for integration

#### 4. Testing Framework
**Timeline**: 2 weeks

**Built-in Test Functions**:
```izi
import { test, assert, assertEqual, assertThrows, assertMatch } from "test";

test("addition works correctly", fn() {
    assertEqual(2 + 2, 4);
    assertEqual(10 + 5, 15);
});

test("division by zero throws", fn() {
    assertThrows(fn() {
        var x = 10 / 0;
    });
});

test("string matching", fn() {
    assertMatch("hello world", "h.*o");
});
```

**Test Runner**:
```bash
izi test                     # Run all tests in tests/
izi test test_math.iz        # Run specific file
izi test --verbose           # Show all output
izi test --watch             # Re-run on changes
izi test --coverage          # Show coverage report
```

**Output**:
```
Running tests...
✓ addition works correctly
✓ division by zero throws
✗ string matching
  AssertionError at test_string.iz:15
  Expected pattern to match, but it didn't

2 passed, 1 failed, 0 skipped
Time: 0.12s
```

**Deliverables**:
- Test discovery and execution
- Assertion library
- Test runner CLI
- Coverage reporting (basic)
- CI integration

#### 5. Package Manager (MVP)
**Timeline**: 4 weeks

**Commands**:
```bash
izi-pkg init                 # Create izi.json
izi-pkg install express      # Install package
izi-pkg install express@1.2  # Install specific version
izi-pkg update               # Update all dependencies
izi-pkg publish              # Publish to registry
izi-pkg search http          # Search for packages
izi-pkg info express         # Show package info
```

**Manifest** (`izi.json`):
```json
{
  "name": "myapp",
  "version": "1.0.0",
  "description": "My awesome app",
  "main": "src/main.iz",
  "dependencies": {
    "express": "^1.0.0",
    "database": "^2.3.0"
  },
  "devDependencies": {
    "test-utils": "^0.5.0"
  }
}
```

**Package Structure**:
```
mypackage/
├── izi.json
├── README.md
├── src/
│   └── index.iz
└── tests/
    └── test_index.iz
```

**Deliverables**:
- Package manager CLI
- Local dependency resolution
- Version constraints (SemVer)
- Package registry backend (simple HTTP API)
- Package search and discovery
- Documentation

#### 6. Debugger Support
**Timeline**: 3 weeks

**Features**:
- Line number tracking in bytecode
- Breakpoint support
- Variable inspection
- Stack frame navigation
- Step in / step over / step out

**VS Code Integration**:
```json
// launch.json
{
  "type": "izi",
  "request": "launch",
  "name": "Debug IziLang",
  "program": "${file}",
  "stopOnEntry": false
}
```

**REPL Debugging**:
```bash
> :debug file.iz
Breakpoint 1 at file.iz:15

> :continue
Stopped at file.iz:15
15 | var result = calculate(x);

> :print x
x = 42

> :step
Stepped into calculate() at file.iz:8

> :vars
x: 42
y: 10

> :continue
Program exited normally
```

**Deliverables**:
- Debug adapter protocol (DAP) server
- VS Code debugger extension
- CLI debugger commands
- Documentation

### Testing & Quality

- 📋 Increase test suite to 150+ test cases
- 📋 Achieve 95%+ code coverage
- 📋 Performance benchmarks (track regressions)
- 📋 Stress testing (memory leaks, long-running)

### Documentation

- 📋 Tutorial: "Build a CLI App in IziLang"
- 📋 Tutorial: "REST API with IziLang"
- 📋 Cookbook with 20+ recipes
- 📋 LSP documentation for editor integration
- 📋 Package manager user guide

### Exit Criteria

v0.4 is complete when:

1. ✅ REPL working with all features
2. ✅ Code formatter production-ready
3. ✅ LSP integrated with VS Code
4. ✅ Test framework functional
5. ✅ Package manager MVP operational
6. ✅ Debugger works in VS Code
7. ✅ All tools documented
8. ✅ 20+ packages published to registry

**Expected Release**: August 31, 2026

---

## v1.0 — Stable (Q4 2026)

**Target Date**: September - December 2026 (4 months)  
**Focus**: Stabilization and polish  
**Status**: Not started

### Objectives

1. Freeze language specification
2. Ensure backward compatibility
3. Polish all features
4. Comprehensive documentation
5. Community building
6. Production validation

### Major Tasks

#### 1. Language Specification Freeze
**Timeline**: 2 weeks

**Deliverables**:
- Complete formal specification document
- Grammar in EBNF notation
- Type system formalization
- Semantics documentation
- Memory model documentation
- Concurrency model (defined, not implemented)

**Frozen Elements**:
- All keywords
- All operators
- Statement and expression syntax
- Standard library API signatures
- Module resolution rules
- Type annotation syntax

**Allowed Post-v1.0**:
- Add new keywords (if not conflicting)
- Add new operators (if not ambiguous)
- Add new standard library functions
- Add new statement types
- Performance improvements
- Bug fixes

**NOT Allowed Post-v1.0 (without major version bump)**:
- Change existing keywords
- Change operator precedence
- Change syntax of existing constructs
- Remove or rename standard library functions
- Change semantics of existing features

#### 2. Semantic Versioning Adoption
**Timeline**: 1 week

**Policy Document**:
```
MAJOR.MINOR.PATCH

MAJOR: Breaking changes (rare, requires migration guide)
MINOR: New features (backward compatible)
PATCH: Bug fixes (backward compatible)
```

**Deprecation Process**:
1. Feature marked deprecated in MINOR version
2. Warning printed at runtime for 1 full MAJOR version
3. Feature removed in next MAJOR version
4. Migration guide provided

**Example Timeline**:
- v1.0: Feature X works normally
- v1.5: Feature X marked deprecated (warning)
- v2.0: Feature X still works (warning)
- v3.0: Feature X removed (with migration guide)

#### 3. Backward Compatibility Testing
**Timeline**: 2 weeks

**Test Suite**:
- Run all v0.2 example programs
- Run all v0.3 example programs
- Run all v0.4 example programs
- Verify no breaking changes

**Compatibility Document**:
```markdown
# v1.0 Breaking Changes

None! v1.0 is 100% backward compatible with v0.4.

# Migration from v0.3
- No breaking changes

# Migration from v0.2
- Classes require `new` keyword (optional in v0.3)
```

#### 4. Performance Optimization
**Timeline**: 4 weeks

**Focus Areas**:
- VM instruction dispatch optimization
- String operations (concat, substring)
- Array operations (push, pop, slice)
- Map operations (insert, lookup)
- Function call overhead

**Benchmarks**:
```
Test Suite | v0.4 | v1.0 Target | Improvement
-----------|------|-------------|------------
Fibonacci  | 2.1s | 1.5s        | 30%
Sorting    | 3.5s | 2.5s        | 29%
JSON parse | 1.8s | 1.2s        | 33%
File I/O   | 0.9s | 0.7s        | 22%
```

**Goal**: Within 2x of Python 3.11 for common operations

#### 5. Production Validation
**Timeline**: 6 weeks (parallel with development)

**Validation Projects**:
1. **CLI Tool** — Build a real command-line utility
2. **REST API** — Build a web service
3. **Data Pipeline** — ETL processing script
4. **Automation Script** — DevOps automation
5. **Game Mod** — Scripting for game engine

**Success Criteria**:
- All projects complete without issues
- Performance acceptable
- No crashes or data corruption
- Good developer experience

#### 6. Documentation Polish
**Timeline**: 3 weeks

**Complete Documentation Set**:

1. **Language Guide** (200+ pages)
   - Getting started
   - Language syntax
   - Standard library
   - Best practices
   - Common patterns

2. **API Reference** (auto-generated)
   - All standard library functions
   - Native function interface
   - Embedding API

3. **Tutorials** (5+)
   - "Build a CLI Tool"
   - "Create a REST API"
   - "Write Tests"
   - "Package and Publish"
   - "Embed IziLang"

4. **Cookbook** (20+ recipes)
   - File operations
   - String manipulation
   - Data structures
   - Error handling
   - Concurrency patterns (when available)

5. **Migration Guides**
   - From Python
   - From JavaScript
   - From Ruby
   - From Go

6. **Reference**
   - Error codes
   - CLI commands
   - Configuration files
   - Environment variables

#### 7. Community Building
**Timeline**: Ongoing

**Infrastructure**:
- Official website (izilang.org)
- Documentation site (docs.izilang.org)
- Package registry (packages.izilang.org)
- GitHub Discussions forum
- Discord server
- Twitter account

**Content**:
- Weekly blog posts
- Monthly newsletter
- Video tutorials
- Conference talks
- Podcast appearances

**Governance**:
- Code of conduct
- Contributing guidelines
- Issue templates
- PR guidelines
- Release process

#### 8. Release Engineering
**Timeline**: 2 weeks

**Release Artifacts**:
- Source tarball
- Binary releases (Linux, macOS, Windows)
- Docker image
- Package manager packages (apt, brew, choco)
- VS Code extension (marketplace)
- Documentation archive

**Release Process**:
1. Feature freeze (2 weeks before release)
2. Release candidate (1 week before)
3. Final testing
4. Version bump
5. Changelog update
6. Tag release
7. Build artifacts
8. Publish to registries
9. Announce on all channels

### Testing & Quality

- 📋 200+ test cases
- 📋 95%+ code coverage
- 📋 Zero known critical bugs
- 📋 Performance within target
- 📋 Security audit
- 📋 Memory leak testing
- 📋 Fuzzing tests

### Success Metrics

**Technical**:
- ✅ All features implemented
- ✅ Test coverage > 95%
- ✅ Performance target met
- ✅ Zero critical bugs
- ✅ Documentation complete

**Community**:
- 🎯 1000+ GitHub stars
- 🎯 50+ packages in registry
- 🎯 100+ active users
- 🎯 10+ contributors
- 🎯 5+ production deployments

### Exit Criteria

v1.0 can be released when:

1. ✅ All v1.0 features complete
2. ✅ Specification frozen and published
3. ✅ Backward compatibility guaranteed
4. ✅ SemVer policy adopted
5. ✅ Production validation successful
6. ✅ Documentation 100% complete
7. ✅ Community infrastructure ready
8. ✅ Release artifacts prepared
9. ✅ Announcement materials ready
10. ✅ Core team confident in stability

**Expected Release**: December 31, 2026 🎉

---

## v1.1 — Productivity (2027 Q1)

**Target Date**: January - March 2027 (3 months)  
**Focus**: Advanced features and productivity enhancements

### Features

#### 1. Class Inheritance
```izi
class Animal {
    var name: string;
    
    fn constructor(name) {
        this.name = name;
    }
    
    fn speak() {
        print(f"{this.name} makes a sound");
    }
}

class Dog extends Animal {
    fn speak() {
        print(f"{this.name} barks");
    }
    
    fn fetch() {
        print(f"{this.name} fetches the ball");
    }
}

var dog = Dog("Buddy");
dog.speak();  // Buddy barks
dog.fetch();  // Buddy fetches the ball
```

#### 2. Type Inference
```izi
// No annotations needed, types inferred
var x = 42;              // Inferred: number
var name = "Alice";      // Inferred: string
var items = [1, 2, 3];   // Inferred: array<number>

fn add(a, b) {           // Inferred: (number, number) -> number
    return a + b;
}
```

#### 3. Advanced Pattern Matching
```izi
// Destructuring in patterns
match person {
    case { name, age: a } if a >= 18 => print(f"{name} is an adult"),
    case { name, age } => print(f"{name} is {age} years old")
}

// Array destructuring
match list {
    case [] => print("empty"),
    case [x] => print(f"one element: {x}"),
    case [first, ...rest] => print(f"first: {first}, rest: {rest}")
}
```

#### 4. Improved Error Recovery
- Parser continues after errors
- Reports multiple errors at once
- Better suggestions
- Did-you-mean suggestions

#### 5. Standard Library Expansion
- std.http (HTTP client)
- std.crypto (hashing, encryption)
- std.path (path manipulation)
- std.os (OS utilities)

---

## v1.2 — Ecosystem (2027 Q2)

**Target Date**: April - June 2027 (3 months)  
**Focus**: Ecosystem growth and platform maturity

### Features

#### 1. Traits / Interfaces
```izi
trait Drawable {
    fn draw();
}

trait Resizable {
    fn resize(width, height);
}

class Circle implements Drawable, Resizable {
    var radius;
    
    fn draw() {
        print("Drawing circle");
    }
    
    fn resize(width, height) {
        this.radius = min(width, height) / 2;
    }
}
```

#### 2. Package Manager Improvements
- Private packages
- Package organizations
- Verified packages
- Package statistics
- Dependency vulnerability scanning

#### 3. FFI (Foreign Function Interface)
```izi
import { ffi } from "ffi";

var libc = ffi.load("libc.so.6");
var strlen = libc.function("strlen", [ffi.string], ffi.int);

print(strlen("hello"));  // 5
```

#### 4. Cross-Platform Native Builds
- Compile to standalone executables
- No runtime dependencies
- Static linking
- Cross-compilation support

#### 5. Web Playground
- Browser-based REPL
- Share code snippets
- Examples gallery
- Interactive tutorial

---

## v2.0 — Advanced (2028+)

**Target Date**: 2028  
**Focus**: Advanced features and breaking improvements

### Features

#### 1. Async/Await Concurrency
```izi
async fn fetchUsers() {
    var response = await http.get("https://api.example.com/users");
    var data = await response.json();
    return data;
}

var users = await fetchUsers();
```

#### 2. JIT Compilation
- Hot path compilation to native
- Profile-guided optimization
- 10x performance improvement for hot loops

#### 3. Advanced Type System
- Generic types
- Union types
- Intersection types
- Type aliases

#### 4. Reflection API
```izi
var type = reflect.type(obj);
var fields = reflect.fields(obj);
var methods = reflect.methods(obj);
```

#### 5. Metaprogramming (Maybe)
- Compile-time code generation
- Macros (hygenic, if added)
- Annotations

---

## Risk Management

### High-Risk Items

#### 1. ⚠️ VM Stability
**Risk**: VM bugs prevent production use  
**Impact**: High — affects performance mode  
**Likelihood**: Medium

**Mitigation**:
- Comprehensive testing
- Fuzzing
- Stress testing
- Community beta testing

**Contingency**: Ship v1.0 with interpreter only, fix VM in v1.1

#### 2. ⚠️ Scope Creep
**Risk**: Too many features delay release  
**Impact**: High — delays v1.0  
**Likelihood**: Medium

**Mitigation**:
- Strict scope control
- Feature prioritization
- Regular reviews
- "No" to non-essential features

**Contingency**: Cut nice-to-have features, focus on must-haves

#### 3. ⚠️ Performance Target
**Risk**: Cannot meet performance goals  
**Impact**: Medium — affects adoption  
**Likelihood**: Low

**Mitigation**:
- Early benchmarking
- Profile-guided optimization
- VM improvements
- Native compilation option

**Contingency**: Adjust expectations, document limitations

#### 4. ⚠️ Community Growth
**Risk**: Low adoption, no community  
**Impact**: Medium — affects ecosystem  
**Likelihood**: Low

**Mitigation**:
- Quality documentation
- Example projects
- Conference talks
- Blog posts

**Contingency**: Focus on niche use cases, grow organically

### Medium-Risk Items

#### 5. ⏸️ Breaking Changes
**Risk**: Need breaking change after v1.0  
**Impact**: Medium — user frustration  
**Likelihood**: Low

**Mitigation**:
- Careful design
- Community feedback before freeze
- Deprecation warnings

**Contingency**: Follow SemVer, provide migration tools

#### 6. ⏸️ Dependency on External Tools
**Risk**: LSP/formatter bugs affect experience  
**Impact**: Low — tooling issues  
**Likelihood**: Medium

**Mitigation**:
- Thorough testing
- Community bug reports
- Rapid bug fixes

**Contingency**: Fall back to basic editor support

---

## Success Metrics

### Version-Specific Metrics

#### v0.3 Success
- ✅ Classes working in 10+ test programs
- ✅ Gradual typing adopted in examples
- ✅ VM passes 100% of tests
- ✅ Standard library has 68+ functions

#### v0.4 Success
- ✅ REPL used in demos
- ✅ LSP working in VS Code
- ✅ 20+ packages published
- ✅ Test framework used in all examples

#### v1.0 Success
- 🎯 1000+ GitHub stars
- 🎯 50+ packages in registry
- 🎯 10+ production deployments
- 🎯 100+ active users
- 🎯 Documentation rated 9/10+

#### v1.1 Success
- 🎯 100+ packages in registry
- 🎯 20+ production deployments
- 🎯 Type inference used in 50% of code

#### v1.2 Success
- 🎯 200+ packages
- 🎯 50+ production deployments
- 🎯 FFI used in 10+ packages

### Overall Success Criteria

**Technical Excellence**:
- Performance within 2x of Python
- Zero critical bugs for 3 months
- 95%+ test coverage maintained
- Documentation completeness 100%

**Community Health**:
- Active contributors (20+)
- Monthly releases
- Responsive maintainers (< 1 week response)
- Positive community sentiment

**Ecosystem Growth**:
- Package growth rate: 10+ per month
- Active packages: 50% (used in last 6 months)
- Production use cases documented
- Success stories published

**Developer Satisfaction**:
- "Easy to learn" rating: 9/10+
- "Good error messages" rating: 9/10+
- "Would recommend" rating: 8/10+
- Stack Overflow questions: 100+

---

## Timeline Summary

```
2026
├── Q1 (Jan-Mar) ← YOU ARE HERE
│   └── [Current] v0.2.0 released
│
├── Q2 (Apr-Jun)
│   ├── Mar: v0.3-alpha (classes, typing)
│   ├── Apr: v0.3-beta (VM stable)
│   └── May: v0.3.0 release
│
├── Q3 (Jul-Sep)
│   ├── Jun: v0.4-alpha (REPL, formatter)
│   ├── Jul: v0.4-beta (LSP, tests)
│   └── Aug: v0.4.0 release
│
└── Q4 (Oct-Dec)
    ├── Sep: v1.0-rc1 (feature freeze)
    ├── Oct: v1.0-rc2 (bug fixes)
    ├── Nov: v1.0-rc3 (polish)
    └── Dec: v1.0.0 STABLE 🎉

2027
├── Q1: v1.1 (inheritance, inference)
├── Q2: v1.2 (traits, FFI)
├── Q3: v1.3 (performance)
└── Q4: v1.4 (polish)

2028+
└── v2.0 (async/await, JIT, breaking improvements)
```

---

## Open Questions

### Must Decide for v0.3

1. **Inheritance Model**
   - Single inheritance only?
   - Multiple inheritance?
   - Mixin approach?
   
   **Recommendation**: Single inheritance, traits for interfaces

2. **Type Inference Scope**
   - Local variables only?
   - Function signatures too?
   - Full program inference?
   
   **Recommendation**: Start with local variables, expand in v1.1

3. **Package Versioning**
   - Strict SemVer?
   - Allow pre-releases?
   - Version pinning?
   
   **Recommendation**: Strict SemVer with ranges (^, ~)

### Must Decide for v1.0

4. **Concurrency Model**
   - Async/await?
   - OS threads?
   - Actor model?
   
   **Recommendation**: Defer to v1.1, choose async/await

5. **Memory Model**
   - Reference counting forever?
   - Switch to GC?
   - Hybrid approach?
   
   **Recommendation**: Evaluate based on v0.3 experience

6. **FFI Design**
   - C only?
   - Python interop?
   - Rust interop?
   
   **Recommendation**: Start with C, expand later

---

## Contributing to the Roadmap

### How to Propose Changes

1. Open GitHub Discussion with `roadmap` label
2. Describe the proposal
3. Explain rationale and impact
4. Discuss alternatives
5. Core team reviews monthly
6. Decisions documented

### Prioritization Criteria

**High Priority**:
- Critical bugs
- Blocking features
- Security issues
- Performance regressions

**Medium Priority**:
- Feature requests (with use cases)
- Documentation improvements
- Tooling enhancements
- Quality of life improvements

**Low Priority**:
- Nice-to-have features
- Experimental features
- Advanced optimizations
- Ecosystem growth

### Decision Making

- **Core team** makes final decisions
- **Community input** heavily weighted
- **User feedback** drives priorities
- **Technical feasibility** considered
- **Alignment with philosophy** required

---

## Conclusion

This roadmap provides a **clear, achievable path** from the current state (v0.2) to a production-ready v1.0 release and beyond. The plan is:

**Realistic**: Based on current progress and team capacity  
**Flexible**: Can adapt to feedback and changing needs  
**Focused**: Prioritizes core features over nice-to-haves  
**Community-Driven**: Incorporates user feedback at every stage

**Key Principles**:
1. **Stability over features** — v1.0 must be rock-solid
2. **Quality over speed** — No rushing, get it right
3. **Community over isolation** — Build together
4. **Evolution over revolution** — Gradual, compatible improvements

**Success** means IziLang becomes a **trusted, productive, and enjoyable** language for everyday programming tasks, true to its promise of being "easy" without sacrificing power.

---

**Document Status**: Living Document  
**Last Updated**: February 2026  
**Next Review**: After v0.3 release  
**Maintained By**: IziLang Core Team  
**Feedback**: GitHub Discussions with `roadmap` label

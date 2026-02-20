# IziLang Development Roadmap

**Last Updated**: February 20, 2026  
**Current Version**: v0.3.0  
**Next Milestone**: v0.4.0 "Concurrency & Ecosystem"

---

## Vision

IziLang aims to be a **modern, expressive, and developer-friendly programming language** that balances simplicity with power. Our goal is to create a language that is:

- **Easy to learn** - Clear syntax, intuitive semantics
- **Powerful** - Rich feature set for real-world applications
- **Fast** - Efficient runtime with multiple execution modes
- **Well-tooled** - Excellent IDE support, debugging, and package management
- **Safe** - Strong error handling, clear diagnostics

---

## Milestone Overview

| Milestone | Target | Status | Description |
|-----------|--------|--------|-------------|
| **v0.1** | Q1 2026 | ✅ Released | "Usable" - Core language functional |
| **v0.2** | Q1 2026 | ✅ Released | "Developer Friendly" - Tooling complete |
| **v0.3** | Q2 2026 | ✅ Released | "Language Power" - OOP, typing, GC |
| **v0.4** | Q3 2026 | 🟡 In Progress | "Concurrency & Ecosystem" |
| **v1.0** | Q4 2026 | ⚪ Future | "Stable" - Backward compatibility guaranteed |

---

## v0.1 — "Usable" ✅ Released (Q1 2026)

**Goal**: Ship a functional language that can run non-trivial programs with clear diagnostics.

### Exit Criteria
- ✅ Can compile and run multi-file programs
- ✅ Clear, helpful error messages with source locations
- ✅ Stable core syntax (no breaking changes planned)
- ✅ Minimal but functional standard library
- ✅ Comprehensive test suite (100% passing)
- ✅ Multi-platform builds (Linux, macOS, Windows)

### Core Language

#### Completed ✅
- [x] Lexer with full token support
- [x] Parser with operator precedence
- [x] AST with visitor pattern
- [x] Tree-walking interpreter (stable, default)
- [x] Bytecode compiler
- [x] All primitive types (nil, bool, number, string)
- [x] Collections (arrays, maps)
- [x] Control flow (if/else, while, for)
- [x] Functions with closures
- [x] Pattern matching
- [x] Module system (import/export)
- [x] Try/catch/finally exception handling
- [x] Break/continue statements
- [x] String interpolation

#### In Progress 🟡
- [ ] **Fix VM bytecode bugs** ⚠️ CRITICAL
  - VM has pre-existing bugs preventing reliable use
  - Needs debugging and test validation
  - Timeline: 1 week
- [ ] **Fix comment parsing** ⚠️ CRITICAL
  - Lexer crashes on `//` comments
  - Simple fix, high impact
  - Timeline: 1 day
- [ ] **Complete wildcard imports** ⚠️ HIGH
  - `import * from "module"` parses but doesn't create namespace objects
  - Needs runtime namespace support
  - Timeline: 3 days

#### Deferred to v0.2
- [ ] Classes/OOP system
- [ ] Ternary operator
- [ ] Type annotations (gradual typing)

### Runtime

#### Completed ✅
- [x] Dual execution model (interpreter + VM)
- [x] Exception handling with try/catch/finally
- [x] Stack traces in errors
- [x] Error reporter with source locations
- [x] Native function interface
- [x] Standard library (46 functions)

#### In Progress 🟡
- [ ] **Decide GC strategy** ⚠️ CRITICAL
  - Currently using reference counting
  - Need explicit decision: mark-sweep, generational, or hybrid
  - Document rationale and implementation plan
  - Timeline: 1 week (decision only)

#### Deferred to v0.3
- [ ] Actual GC implementation
- [ ] Performance optimizations
- [ ] Concurrency support

### Tooling

#### Completed ✅
- [x] Basic CLI (`izi <file>`)
- [x] Build system (Premake5)
- [x] CI/CD pipeline (GitHub Actions)
- [x] Test framework (Catch2)
- [x] Multi-platform support

#### v0.1 Scope
- [ ] Improve CLI help messages
- [ ] Add `--version` flag
- [ ] Better error messages for file not found

#### Deferred to v0.2
- [ ] REPL (interactive mode)
- [ ] Code formatter (`izi fmt`)
- [ ] LSP server implementation
- [ ] Package manager backend

### Documentation

#### Completed ✅
- [x] README with build instructions
- [x] Architecture documentation (ANALYSIS.md)
- [x] Feature summaries (17 markdown files)
- [x] Example programs

#### v0.1 Scope
- [x] **STATUS.md** - Current project state
- [x] **ROADMAP.md** - This document
- [x] **DECISIONS.md** - Frozen design decisions
- [ ] Language specification (basic)
- [ ] Tutorial (getting started)

### Testing

#### Completed ✅
- [x] 63 test cases, 273 assertions
- [x] Unit tests (lexer, value types)
- [x] Integration tests (end-to-end)
- [x] All tests passing (100%)

#### v0.1 Scope
- [ ] Add edge case tests
- [ ] Test error recovery
- [ ] Benchmark suite (basic)

---

## v0.2 — "Developer Friendly" ✅ Released (Q1 2026)

**Goal**: Deliver a complete developer experience with excellent tooling.

### Exit Criteria
- ✅ Interactive REPL for experimentation
- ✅ Code formatter for consistent style
- ✅ LSP server with basic features
- ✅ Package manager with local dependencies
- ✅ Tutorial and learning resources

### Core Language

#### Features
- [ ] **Classes and OOP** ⚠️ HIGH PRIORITY
  - Class declarations
  - Instance methods and properties
  - Inheritance (single)
  - Constructors
  - `this` keyword
  - Timeline: 2-3 weeks

- [ ] **Ternary Operator**
  - Syntax: `condition ? true_expr : false_expr`
  - Parser and interpreter support
  - Timeline: 2 days

- [ ] **Gradual Typing** (Optional)
  - Type annotations: `var x: number = 42;`
  - Function signatures: `fn add(a: number, b: number): number`
  - Runtime checks (no static analysis yet)
  - Timeline: 2 weeks

- [ ] **Improved Error Recovery**
  - Parser continues after errors
  - Reports multiple errors at once
  - Timeline: 1 week

### Tooling

#### REPL (Interactive Mode)
- [ ] `izi repl` command
- [ ] Multi-line input support
- [ ] History navigation (up/down arrows)
- [ ] Syntax highlighting
- [ ] Auto-completion (basic)
- Timeline: 1 week

#### Code Formatter
- [ ] `izi fmt <file>` command
- [ ] Format entire directory: `izi fmt src/`
- [ ] Check mode: `izi fmt --check`
- [ ] Configurable style (`.izifmt.toml`)
- [ ] Auto-indent, spacing rules
- Timeline: 2 weeks

#### LSP Server
- [ ] **Diagnostics** (syntax errors, warnings)
- [ ] **Auto-completion** (variables, functions, keywords)
- [ ] **Hover information** (type info, documentation)
- [ ] **Go-to-definition**
- [ ] **Find references**
- [ ] **Rename symbol**
- Timeline: 4 weeks

#### VS Code Extension
- [ ] Connect to LSP server
- [ ] Debugger support (basic)
- [ ] Syntax highlighting (improved)
- [ ] Code snippets
- [ ] Marketplace publication
- Timeline: 2 weeks

#### Package Manager (MVP)
- [ ] `izi-pkg init` - Initialize project
- [ ] `izi-pkg install <package>` - Install dependency
- [ ] `izi-pkg publish` - Publish to registry
- [ ] Local dependency resolution
- [ ] `izi.json` manifest file
- [ ] Registry backend (simple HTTP API)
- Timeline: 3 weeks

### Standard Library Expansion

#### New Modules
- [ ] **std.json** - JSON parsing and serialization
  - `parse(string)`, `stringify(value)`
- [ ] **std.regex** - Regular expressions
  - `match(pattern, string)`, `replace(pattern, string, replacement)`
- [ ] **std.time** - Date and time utilities
  - `now()`, `sleep(ms)`, `format(timestamp)`
- Timeline: 2 weeks

### Documentation

- [ ] **Language Specification** (formal)
  - Grammar (EBNF)
  - Type system
  - Semantics
  - Timeline: 2 weeks

- [ ] **Tutorial** (interactive)
  - "Learn IziLang in Y minutes"
  - Step-by-step guide
  - Exercises
  - Timeline: 1 week

- [ ] **API Reference** (generated)
  - All stdlib functions documented
  - Examples for each function
  - Timeline: 1 week

- [ ] **Cookbook** (practical examples)
  - Common patterns
  - Best practices
  - Real-world examples
  - Timeline: 1 week

### Testing

- [ ] Increase test coverage to 90%+
- [ ] Add stress tests
- [ ] Add fuzzing tests
- [ ] Performance benchmarks

---

## v0.3 — "Language Power" ✅ Released (Q2 2026)

**Goal**: Optimize for production use with performance, stability, and debugging.

### Exit Criteria
- ✅ Performance comparable to interpreted languages (Python, Ruby)
- ✅ Concurrency model implemented and documented
- ✅ Full debugging support (breakpoints, stepping)
- ✅ Cross-platform native builds
- ✅ Large-scale projects run reliably

### Performance

#### Optimization Passes
- [ ] **Constant Folding**
  - Evaluate constant expressions at compile time
  - `2 + 3` → `5`
  - Timeline: 1 week

- [ ] **Dead Code Elimination**
  - Remove unreachable code
  - Eliminate unused variables
  - Timeline: 1 week

- [ ] **Inline Caching**
  - Speed up property lookups
  - Cache function call targets
  - Timeline: 2 weeks

- [ ] **JIT Compilation** (Optional, Stretch Goal)
  - Compile hot paths to native code
  - Requires significant effort
  - Timeline: 6-8 weeks

#### Benchmarking
- [ ] Comprehensive benchmark suite
- [ ] Compare against Python, Ruby, JavaScript
- [ ] Track performance over time
- [ ] Publish results
- Timeline: 1 week

### Concurrency

**Model Decision**: Async/Await with Event Loop

#### Features
- [ ] **Async Functions**
  - Syntax: `async fn fetch(url) { ... }`
  - `await` keyword for async calls
  - Timeline: 3 weeks

- [ ] **Promises/Futures**
  - `Promise<T>` type
  - `then()`, `catch()`, `finally()` methods
  - Timeline: 2 weeks

- [ ] **Event Loop**
  - Non-blocking I/O
  - Task scheduler
  - Timeline: 2 weeks

- [ ] **Concurrent Collections**
  - Thread-safe arrays and maps
  - Timeline: 1 week

- [ ] **Standard Library Support**
  - `std.async` module
  - Async file I/O, HTTP client
  - Timeline: 2 weeks

### Garbage Collection

**Strategy Decision**: Mark-and-Sweep with Generational Extension

#### Implementation
- [ ] **Mark-and-Sweep GC**
  - Stop-the-world collection
  - Root set identification
  - Timeline: 3 weeks

- [ ] **Generational GC** (Optional)
  - Young generation (frequent collection)
  - Old generation (infrequent collection)
  - Timeline: 2 weeks

- [ ] **Tuning Parameters**
  - Heap size limits
  - Collection thresholds
  - Timeline: 1 week

### Debugging Support

#### Debugger Protocol
- [ ] Debug Adapter Protocol (DAP) implementation
- [ ] Breakpoints (line-based)
- [ ] Stepping (step in, step over, step out)
- [ ] Variable inspection
- [ ] Call stack examination
- [ ] Timeline: 4 weeks

#### VS Code Debugger
- [ ] Debug configurations
- [ ] Launch and attach modes
- [ ] Integrated terminal
- [ ] Timeline: 2 weeks

### Cross-Platform Builds

#### Native Compilation
- [ ] Compile to standalone executables
- [ ] No runtime dependencies
- [ ] Static linking
- [ ] Timeline: 2 weeks

#### Distribution
- [ ] Package managers (apt, brew, chocolatey)
- [ ] Docker images
- [ ] Timeline: 1 week

### Testing & Stability

- [ ] Stress testing (long-running programs)
- [ ] Memory leak detection
- [ ] Crash reporting
- [ ] Production monitoring

---

## v0.4 — "Concurrency & Ecosystem" 🟡 In Progress (Target: Q3 2026)

**Goal**: Add concurrency support, package manager, debugger, and LSP server.

### Core Language

- [ ] **Async/Await** - Async functions with event loop
- [ ] **Promises** - `then()`, `catch()`, `finally()` chaining
- [ ] **Concurrent Collections** - Thread-safe arrays and maps

### Tooling

- [ ] **Code Formatter** - `izi fmt` command with configurable style
- [ ] **LSP Server** - Diagnostics, auto-completion, hover, go-to-definition
- [ ] **VS Code Extension** - Connect to LSP, debugger support, marketplace
- [ ] **Package Manager** - `izi-pkg init/install/publish` with registry
- [ ] **Debug Adapter Protocol** - Breakpoints, stepping, variable inspection

### Standard Library

- [ ] **std.async** - Async I/O, HTTP client
- [ ] **std.json** (already available via native modules) - Stabilize API

### Performance

- [ ] **Constant Folding** - Evaluate constant expressions at compile time
- [ ] **Inline Caching** - Speed up property lookups
- [ ] **Benchmark Suite** - Compare against Python, Ruby, JavaScript

---

## v1.0 — "Stable" (Target: Q4 2026)

**Goal**: Declare the language stable with backward compatibility guarantees.

### Exit Criteria
- ✅ Semantic versioning adopted
- ✅ No breaking changes without deprecation cycle
- ✅ Production deployments validated
- ✅ Community ecosystem emerging
- ✅ Comprehensive documentation

### Language Freeze

- [ ] **Syntax Freeze** - No breaking syntax changes
- [ ] **Semantic Freeze** - Behavior locked
- [ ] **Stdlib Freeze** - API stability guaranteed

### Deprecation Policy

- [ ] Deprecation warnings for 1 major version
- [ ] Removal after 2 major versions
- [ ] Clear migration guides

### Community

- [ ] Public registry for packages
- [ ] Community forums/Discord
- [ ] Contribution guidelines
- [ ] Code of conduct
- [ ] Governance model

### Ecosystem

- [ ] 50+ packages in registry
- [ ] 10+ production deployments
- [ ] Active contributor base
- [ ] Tutorials and courses
- [ ] Conference talks

---

## Post-v1.0 — Future Directions

### Potential Features (Not Committed)

#### Advanced Type System
- Static type checking (optional)
- Type inference
- Generics/templates
- Algebraic data types

#### Performance
- JIT compilation
- WASM target
- GPU computation support

#### Concurrency
- Parallel collections
- Actor model
- Software transactional memory

#### Metaprogramming
- Macros
- Compile-time code generation
- Reflection API

#### Interoperability
- C FFI (foreign function interface)
- JavaScript interop (for web)
- Python interop

---

## Release Process

### Version Numbering (SemVer)

```
MAJOR.MINOR.PATCH

MAJOR: Breaking changes
MINOR: New features, backward compatible
PATCH: Bug fixes, backward compatible
```

### Release Cycle

- **Patch Releases**: As needed (bug fixes)
- **Minor Releases**: Every 1-2 months (new features)
- **Major Releases**: Every 6-12 months (breaking changes)

### Pre-Release Stages

1. **Alpha** - Feature development (unstable API)
2. **Beta** - Feature complete (API may change)
3. **RC** - Release candidate (API frozen)
4. **Stable** - Production ready

### Release Checklist

- [ ] All tests passing
- [ ] Documentation updated
- [ ] Changelog written
- [ ] Migration guide (if breaking changes)
- [ ] Binaries built for all platforms
- [ ] GitHub release created
- [ ] Announcement posted
- [ ] Package managers updated

---

## Contributing to the Roadmap

### How to Propose Features

1. Open an issue with label `feature-request`
2. Describe the use case
3. Provide examples
4. Discuss alternatives
5. Wait for community feedback

### Prioritization Criteria

1. **Impact** - How many users benefit?
2. **Effort** - How long to implement?
3. **Risk** - What could go wrong?
4. **Alignment** - Does it fit the vision?

### Decision Making

- Core team reviews proposals monthly
- Community input via GitHub discussions
- Final decisions documented in DECISIONS.md

---

## Timeline Summary

```
Q1 2026 ✅ Completed
├── v0.1.0 Release - "Usable" (core language, interpreter, VM, stdlib)
└── v0.2.0 Release - "Developer Friendly" (CLI, REPL, error messages)

Q2 2026 ✅ Completed
├── v0.3.0 Release - "Language Power"
│   ├── Classes and OOP with inheritance
│   ├── Gradual typing system
│   ├── Mark-and-Sweep garbage collection
│   ├── Semantic analysis (izi check)
│   ├── Stack overflow protection
│   └── VM import system and benchmarks

Q3 2026 🟡 In Progress
├── Code formatter (izi fmt)
├── LSP server (basic)
├── Async/await concurrency
├── Package manager (MVP)
└── v0.4.0 Release - "Concurrency & Ecosystem"

Q4 2026
├── Stability improvements
├── Ecosystem growth
├── Documentation polish
├── Production validation
└── v1.0.0 Release - "Stable"
```

---

## Success Metrics

### v0.1
- ✅ 100% test pass rate
- ✅ Build on 3 platforms
- ✅ 10+ example programs
- ✅ 5+ contributors

### v0.2
- ✅ REPL working
- ✅ CLI expansion (run, build, check, test, repl, bench)
- ✅ Enhanced error messages with source context

### v0.3
- ✅ 835 assertions, 142 test cases, 100% passing
- ✅ Classes and OOP with full inheritance
- ✅ Gradual typing system (optional annotations)
- ✅ Mark-and-Sweep garbage collection
- ✅ Semantic analysis and static checks

### v0.4
- 🎯 LSP server in VS Code
- 🎯 Code formatter
- 🎯 Async/await concurrency
- 🎯 Package manager MVP

### v1.0
- 🎯 100+ packages in registry
- 🎯 10+ production deployments
- 🎯 50+ contributors
- 🎯 1000+ GitHub stars

---

## Risk Management

### Known Risks

1. **Concurrency Complexity** - Async/await with event loop is non-trivial
   - Mitigation: Implement incrementally; ship green-thread model first
   - Fallback: Defer to v0.5 if needed

2. **LSP Server** - Full LSP implementation requires significant effort
   - Mitigation: Ship diagnostics-only MVP first, expand iteratively
   - Fallback: Provide basic syntax highlighting via TextMate grammar

3. **Scope Creep** - Too many features
   - Mitigation: Strict milestone adherence
   - Fallback: Cut features, not quality

4. **Performance** - May not meet targets
   - Mitigation: Profile and optimize iteratively
   - Fallback: Adjust expectations, document limitations

5. **Community Growth** - May not attract users
   - Mitigation: Focus on documentation, examples
   - Fallback: Niche focus (education, scripting)

---

## Conclusion

This roadmap provides a **clear path** from the current state to a production-ready language. The focus is on:

1. **Stability first** - Fix bugs before adding features
2. **Incremental progress** - Small, shippable milestones
3. **Community driven** - Feedback shapes priorities
4. **Quality over quantity** - Well-tested, documented features

**Next Steps**: Complete v0.4 with code formatter, LSP server, and async/await concurrency.

---

**Document maintained by**: IziLang Development Team  
**Feedback**: Open an issue or discussion on GitHub

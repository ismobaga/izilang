# IziLang Project Analysis, Roadmap & Feature Definition (v1.x)

**Document Version**: 1.0  
**Last Updated**: February 2026  
**Status**: Living document — open for community input  

---

## Table of Contents

1. [Project Identity](#1-project-identity)
2. [Core Language Audit](#2-core-language-audit)
3. [Feature Inventory](#3-feature-inventory)
4. [v1.0 Scope Definition](#4-v10-scope-definition-mvp)
5. [Roadmap Proposal](#5-roadmap-proposal)
6. [Feature Suggestion Guidelines](#6-feature-suggestion-guidelines)
7. [Open Questions](#7-open-questions)
8. [Risks & Trade-offs](#8-risks--trade-offs)

---

## 1. Project Identity

### 1.1 Language Manifesto

**izi** is named for how it sounds — *easy* — and this principle is its north star.

izi is a **modern, multi-paradigm scripting language** that prioritises:

- **Clarity over cleverness** — code should read like intent, not like a puzzle.
- **Predictable semantics** — no implicit coercion, no surprising corner cases.
- **Practical defaults** — batteries-included standard library, excellent errors.
- **Incremental sophistication** — start simple, add types and classes as projects grow.

izi is not trying to replace systems languages. It is trying to be the language you reach for when you want to write something real — a tool, a server, an automation script — without fighting the language itself.

### 1.2 Positioning Statement

> **IziLang is an easy-to-learn, expressive scripting language for developers who want Python's readability, JavaScript's flexibility, and Rust's error quality — without the complexity of any of them.**

### 1.3 Target Audience

| Audience | Use Case |
|----------|----------|
| Intermediate developers | Backend services, automation, tooling |
| Students | Learning programming concepts without ceremony |
| Scripters | Replacing bash/Python for lightweight tasks |
| Educators | Teaching language concepts with a real interpreter |

izi is **not** aimed at:

- Systems programmers needing manual memory control (use C/C++/Rust)
- Data scientists needing a rich numeric ecosystem (use Python)
- Embedded developers with hard real-time constraints (use C)

### 1.4 Non-Goals

The following are explicitly **out of scope** for izi v1.x:

| Non-Goal | Rationale |
|----------|-----------|
| Manual memory management | Safety is a core value; `malloc`/`free` is never exposed |
| Compile-to-native (AOT) as default | Simplicity of distribution wins; `izi compile` is opt-in |
| Pure functional enforcement | Immutability is good but not mandated |
| Type system as primary identity | Gradual typing supplements, not defines, the language |
| Competing with Python/JS ecosystems | izi has its own identity; ecosystem comes later |
| Logic / reactive programming models | Outside the imperative + functional scope |
| WASM / GPU targets | Deferred to v2.x if validated by real demand |

---

## 2. Core Language Audit

### 2.1 Strengths

| Area | Assessment | Notes |
|------|------------|-------|
| Lexer | ✅ Excellent | Full token support, comments, interpolation, error locations |
| Parser | ✅ Good | Recursive descent, precedence climbing, clear errors |
| AST Design | ✅ Excellent | Visitor pattern, clean separation, easy to extend |
| Tree-walking interpreter | ✅ Stable | Default execution path, all tests passing |
| Error messages | ✅ Production-quality | Rust-style carets, descriptive messages, source locations |
| Module system | ✅ Complete | Named and wildcard imports, namespace objects, circular-safe |
| Standard library | ✅ Good | 46 native functions across math, string, array, io |
| Exception handling | ✅ Complete | try/catch/finally, stack traces, custom error values |
| Collections | ✅ Complete | Arrays, maps with rich operations |
| Pattern matching | ✅ Complete | match expressions, guards, wildcards |
| String interpolation | ✅ Complete | f-strings with `${}` expression embedding |
| Classes / OOP | ✅ Added in v0.3 | Single inheritance, constructors, `this`, property access |
| Gradual typing | ✅ Added in v0.3 | Optional annotations, `izi check` static validation |
| GC | ✅ Added in v0.3 | Mark-and-Sweep replaces reference counting |
| Build system | ✅ Stable | Premake5, multi-platform, fast |
| CI/CD | ✅ Production-grade | GitHub Actions, multi-platform, automated releases |
| Test suite | ✅ Comprehensive | 100% pass rate, Catch2 |

### 2.2 Gaps and Weaknesses

| Area | Status | Target |
|------|--------|--------|
| Bytecode VM | ⚠️ Experimental | v0.4 — fix remaining edge cases, make production-ready |
| LSP server | ⚠️ Framework only | v0.4 — diagnostics, completion, hover |
| Code formatter | ❌ Not implemented | v0.4 — `izi fmt` command |
| Async / concurrency | ❌ Not implemented | v0.4 — async/await with event loop |
| Package manager | ⚠️ Spec only | v0.4 — `izi-pkg` MVP with local deps |
| Debugger (DAP) | ❌ Not implemented | v0.5 — Debug Adapter Protocol |
| Type inference | ❌ Not implemented | v0.5 — infer types without annotations |
| REPL polish | ⚠️ Basic | v0.4 — dot notation, history, completion |
| Dot notation for modules | ❌ Not implemented | v0.4 — `math.sqrt()` instead of `math["sqrt"]()` |
| `const` bindings | ❌ Not implemented | v1.0 — immutable `const` declarations |
| Error recovery in parser | ⚠️ Basic | v0.4 — report multiple errors per run |
| Performance profiling | ❌ No tooling | v0.4 — `izi bench` improvements |

### 2.3 Design Decision Summary

All critical design decisions are frozen and documented in [`docs/DECISIONS.md`](DECISIONS.md).
Key decisions for reference:

| Decision | Choice | Frozen |
|----------|--------|--------|
| Execution model | Interpreter default, VM experimental | 🔒 v0.1 |
| Type system | Dynamic now, gradual (optional) from v0.3 | 🔒 v0.3 |
| Memory management | Mark-and-Sweep GC (from v0.3) | 🔒 v0.3 |
| Error handling | try/catch/finally exceptions | 🔒 v0.1 |
| Module system | ES6-style import/export | 🔒 v0.1 |
| Semicolons | Optional | 🔒 v0.1 |
| Numbers | 64-bit float (no separate int) | 🔒 v0.1 |
| No implicit coercion | Explicit casts only, except truthiness | 🔒 v0.1 |
| Backward compat | Breaking changes allowed pre-v1.0; SemVer after | 🔒 v0.1 |

---

## 3. Feature Inventory

### 3.1 Current Features (v0.3)

#### Core Language
- [x] Primitive types: nil, bool, number, string
- [x] Collections: array, map
- [x] Arithmetic, comparison, and logical operators
- [x] Variable declarations (`var`)
- [x] Control flow: if/else, while, for, break, continue
- [x] Functions with closures and recursion
- [x] First-class functions (pass, return, store)
- [x] Pattern matching (`match`/`case`)
- [x] Exception handling (try/catch/finally)
- [x] String interpolation (f-strings)
- [x] Comments (single-line `//`, multi-line `/* */`)
- [x] Classes with single inheritance
- [x] Constructors and `this` binding
- [x] Optional type annotations
- [x] `izi check` for static semantic analysis

#### Module System
- [x] Named imports: `import { fn } from "module"`
- [x] Wildcard imports: `import * as m from "module"`
- [x] Exports: `export var`, `export fn`
- [x] Native stdlib modules: math, string, array, io
- [x] Circular import safety

#### Runtime
- [x] Tree-walking interpreter (default, production-ready)
- [x] Bytecode VM (experimental, `--vm` flag)
- [x] Mark-and-Sweep garbage collection
- [x] Stack overflow protection (max depth: 256)
- [x] Detailed error messages with source locations
- [x] Stack traces with function names and line numbers

#### Tooling
- [x] `izi run` — execute scripts
- [x] `izi build` — compile (check) without running
- [x] `izi check` — static semantic analysis
- [x] `izi test` — run test files
- [x] `izi repl` — interactive REPL
- [x] `izi bench` — performance benchmarking
- [x] `izi compile` — native compilation (experimental)
- [x] Multi-platform builds (Linux, macOS, Windows)
- [x] CI/CD pipeline (GitHub Actions)
- [x] VS Code extension (syntax highlighting)

#### Standard Library (46 functions)
- [x] `std.math` — sqrt, pow, trig, floor/ceil/round, min/max, PI, E
- [x] `std.string` — substring, split, trim, replace, indexOf, join, case conversion
- [x] `std.array` — map, filter, reduce, sort, reverse, concat, slice
- [x] `std.io` — readFile, writeFile, appendFile, fileExists
- [x] `std.ipc` — named-pipe inter-process communication (POSIX)

### 3.2 Missing / Planned Features

#### High Priority (v0.4)
- [ ] `izi fmt` — code formatter with configurable style
- [ ] LSP server — diagnostics, completion, hover, go-to-definition
- [ ] Async/await with event loop
- [ ] Dot notation for module objects: `math.sqrt(16)`
- [ ] Import aliasing: `import { sqrt as sqrtFn } from "math"`
- [ ] `std.json` — JSON parse/stringify
- [ ] `std.time` — date/time utilities
- [ ] Package manager MVP (`izi-pkg`)

#### Medium Priority (v0.5)
- [ ] Debug Adapter Protocol (DAP) — breakpoints, stepping, inspection
- [ ] Type inference — deduce types without explicit annotations
- [ ] `const` keyword — immutable bindings
- [ ] Visibility modifiers — `public`/`private` on class members
- [ ] `std.regex` — regular expressions
- [ ] `std.http` — HTTP client
- [ ] Improved error recovery — report multiple errors per parse run
- [ ] Ternary operator — `condition ? a : b`
- [ ] Default function parameters — `fn f(x = 0) { ... }`

#### Lower Priority (v1.0 polish)
- [ ] Interfaces / protocols — structural typing for classes
- [ ] Enum types — named constant sets
- [ ] Spread/rest operators — `fn f(...args)`, `[...a, ...b]`
- [ ] Destructuring assignment — `var [a, b] = arr`
- [ ] Generator functions — `yield`
- [ ] `std.async` — async I/O primitives
- [ ] Package registry backend — public registry for community packages
- [ ] Homebrew/apt/winget distribution

---

## 4. v1.0 Scope Definition (MVP)

### 4.1 v1.0 Principle

**v1.0 means**: the language is stable. Programs written for v1.0 will run unchanged on v1.1, v1.2, and v2.0 without warnings.

### 4.2 v1.0 Feature List

The following must be complete, tested, and documented before a v1.0 tag:

#### Language (must-have)
- [x] All primitive types (nil, bool, number, string)
- [x] Collections (array, map) with full operations
- [x] Complete control flow (if/else, while, for, break, continue)
- [x] First-class functions with closures
- [x] Classes with single inheritance, constructors, `this`
- [x] Pattern matching
- [x] Exception handling (try/catch/finally)
- [x] Module system (import/export, stdlib)
- [x] String interpolation
- [ ] Dot notation for module/object access (`obj.method()`)
- [ ] `const` immutable bindings
- [ ] Default function parameters
- [ ] Ternary operator

#### Runtime (must-have)
- [x] Garbage collection (Mark-and-Sweep, v0.3)
- [x] Stack overflow protection
- [x] Detailed error messages with source locations
- [ ] Production-ready bytecode VM as default or stable option
- [ ] Stable async/await with event loop

#### Tooling (must-have)
- [x] `izi run` / `izi repl` / `izi check` / `izi test`
- [ ] `izi fmt` — code formatter
- [ ] LSP server — diagnostics + completion (minimum viable)
- [ ] Package manager — `izi-pkg init/install/publish`

#### Standard Library (must-have)
- [x] std.math, std.string, std.array, std.io
- [ ] std.json — JSON parsing/serialization
- [ ] std.time — date/time utilities
- [ ] std.regex — regular expressions (basic)
- [ ] std.http — HTTP client (basic GET/POST)

#### Quality (must-have)
- [ ] 90%+ test coverage
- [ ] All features documented with examples
- [ ] Language specification (EBNF grammar)
- [ ] Migration guide from v0.x to v1.0

#### Stability Guarantee (must-have)
- [ ] SemVer strictly enforced
- [ ] No breaking syntax changes after v1.0 RC1
- [ ] Deprecation policy in place (1-version warning before removal)
- [ ] Changelog complete and accurate

### 4.3 What v1.0 Will NOT Include

These are deferred to v1.x or v2.0 to keep the scope manageable:

- Static type inference (v1.x stretch goal)
- JIT compilation (v1.x/v2.0)
- WASM target (v2.0)
- Advanced generics / algebraic types (v2.0)
- C FFI (v1.x stretch goal)
- GUI / UI frameworks (ecosystem, not core)
- Parallel collections / actor model (v2.0)

---

## 5. Roadmap Proposal

### Phase 1 — Foundation (v0.4, Q3 2026)

**Theme**: Developer Productivity  
**Goal**: Make izi delightful to use every day

| Feature | Priority | Notes |
|---------|----------|-------|
| `izi fmt` code formatter | High | Configurable style, `--check` mode |
| LSP server (diagnostics + completion) | High | VS Code first, then generic |
| Async/await with event loop | High | Non-blocking I/O |
| Dot notation for modules | High | `math.sqrt()` — quality-of-life |
| Import aliasing | Medium | `import { fn as alias }` |
| `std.json` | Medium | Blocking ask from users |
| Package manager MVP | Medium | `izi-pkg init/install` |
| Error recovery in parser | Medium | Report multiple errors |

**Exit Criteria**: Every izi developer can write, format, and check izi code in VS Code with zero friction.

---

### Phase 2 — Productivity (v0.5, Q4 2026)

**Theme**: Language Completeness  
**Goal**: Remove rough edges, add missing language features

| Feature | Priority | Notes |
|---------|----------|-------|
| Debug Adapter Protocol | High | Breakpoints in VS Code |
| Type inference | High | No annotation required for common cases |
| `const` bindings | High | Immutability where intended |
| Default function parameters | Medium | `fn f(x = 0)` |
| Ternary operator | Medium | `a ? b : c` |
| `std.regex` | Medium | Essential for text processing |
| `std.http` | Medium | Basic HTTP client |
| Visibility modifiers | Medium | `public`/`private` for classes |
| Interfaces / protocols | Low | Structural typing for classes |

**Exit Criteria**: izi can be used for production backend services and tools without workarounds.

---

### Phase 3 — Ecosystem (v1.0 RC, Q1 2027)

**Theme**: Stability and Community  
**Goal**: Declare the language stable; grow the ecosystem

| Feature | Priority | Notes |
|---------|----------|-------|
| SemVer enforcement | Critical | No breaking changes from RC1 |
| Package registry backend | High | Public registry for community packages |
| Language specification (EBNF) | High | Formal grammar document |
| 90%+ test coverage | High | Every feature tested |
| Distribution (brew/apt/winget) | Medium | Easy installation |
| Community forum / Discord | Medium | Gather users and contributors |

**Exit Criteria**: Third-party developers can publish packages; v1.0 is tagged and announced.

---

### Phase 4 — Advanced (v1.x+, 2027+)

**Theme**: Power Features  
**Goal**: Grow izi's capabilities for advanced use cases  
**Commitment**: None of these will break v1.0 code

| Feature | Notes |
|---------|-------|
| JIT compilation | Hot-path native code generation |
| Static type inference | No annotations needed; errors at compile time |
| C FFI | Call C libraries from izi |
| WASM target | Run izi in browsers |
| Enum types | Named constant sets with exhaustive match |
| Generator functions | `yield`-based iterators |
| Parallel collections | Thread-safe arrays/maps |
| Actor model | Message-passing concurrency (v2.0) |

---

## 6. Feature Suggestion Guidelines

### 6.1 How to Propose a Feature

1. **Open a GitHub Issue** with the `feature-request` label
2. **Title format**: `[Feature] Short description`
3. **Required sections** in the issue body:
   - **Problem**: What real use case does this solve?
   - **Proposed syntax / API**: What would it look like?
   - **Example code**: At least one working example
   - **Alternatives considered**: What workarounds exist today?
   - **Breaking changes**: Would this break existing code?

### 6.2 Prioritization Criteria

Features are prioritized using these factors (in order):

| Factor | Weight | Description |
|--------|--------|-------------|
| **User impact** | 40% | How many users need this? Can they work around it? |
| **Language coherence** | 25% | Does it fit izi's identity and existing design? |
| **Implementation effort** | 20% | Days to implement well, including tests and docs |
| **Risk** | 15% | Could it introduce bugs, breaking changes, or ambiguity? |

### 6.3 Fast-Track Criteria

A feature may be fast-tracked (skipping the usual discussion period) if:
- It is a pure **bug fix** with no semantic change
- It is a **stdlib addition** (new function, no breaking change)
- It has **unanimous core team support** and zero breaking changes

### 6.4 Rejection Reasons

A feature will be rejected if:
- It conflicts with a **frozen design decision** (see `docs/DECISIONS.md`)
- It introduces **implicit behavior** that violates the "no surprises" principle
- It is better served by a **library or package** than a language feature
- It is **out of scope** (see Non-Goals in Section 1.4)

---

## 7. Open Questions

The following decisions have not yet been made and need community input:

### Q1: Concurrency Model — Threads vs. Actors?

**Context**: v0.4 targets async/await. For v1.x, should izi also support true parallelism?  
**Options**:
- A) Async/await only (single-threaded event loop, like Node.js)
- B) Async/await + OS threads for CPU-bound work (like Dart)
- C) Actor model for all concurrency (like Elixir, Erlang)
- D) No parallel execution until v2.0

**Input needed**: Real-world use cases from the community. Open a discussion with the `concurrency` label.

---

### Q2: Default Export Syntax?

**Context**: Current module system supports named and wildcard exports. Should we add default exports?  
**Proposal**: `export default fn ...` / `import myFn from "module"`  
**Concern**: Less explicit, potential naming confusion  
**Decision**: Defer to v0.5 pending community feedback

---

### Q3: Integer Type?

**Context**: Numbers are 64-bit floats (like JavaScript/Lua). This causes precision loss beyond 2^53.  
**Options**:
- A) Keep single `number` type (current)
- B) Add `int` type distinct from `float`
- C) Add `bigint` for arbitrary precision integers

**Decision criterion**: Depends on target use cases. If izi is used for finance or cryptography, bigint becomes critical. Open a discussion with the `types` label.

---

### Q4: Visibility Modifiers for Classes?

**Context**: Classes currently have no `public`/`private` concept.  
**Options**:
- A) Convention-based (`_name` = private, no enforcement)
- B) Keyword-based (`private var name`)
- C) Structural: all properties accessible, encapsulation by module boundary

---

### Q5: Naming — `.iz` vs `.izi`?

**Context**: The repository uses both `.iz` and `.izi` file extensions.  
**Decision needed**: Standardize on one extension for v1.0.  
**Recommendation**: Standardize on `.iz` (shorter, already dominant in examples).

---

## 8. Risks & Trade-offs

### 8.1 Risk Register

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| **Concurrency complexity** — async/await with event loop is non-trivial in C++ | Medium | High | Ship minimal async primitives first; grow incrementally |
| **LSP scope creep** — full LSP implementation is 8+ weeks | Medium | Medium | Ship diagnostics-only MVP; iterate from user feedback |
| **VM stability** — bytecode VM has known edge cases | Low | High | Keep interpreter as default; VM opt-in until fully tested |
| **Scope creep to v1.0** — too many features delay the stable tag | High | High | Hard feature freeze for v1.0 RC; defer non-essentials |
| **Community growth** — izi may not attract users before ecosystem exists | Medium | High | Focus on documentation, examples, and a compelling story |
| **Circular reference leaks** — GC handles cycles but correctness matters | Low | Medium | Expand GC stress tests; add cycle detection tooling |
| **Backward compatibility breaks** — accidental API changes | Low | High | Comprehensive changelog; feature flags before removal |

### 8.2 Key Trade-offs

#### Trade-off 1: `number` as float vs. separate int type

| Side | Argument |
|------|----------|
| **Float only (current)** | Simpler language, no implicit conversions, familiar to JS/Lua users |
| **Separate int type** | Precision for large numbers, clearer semantics for integers |

**Current stance**: Float-only for v1.0. Revisit only if community reports concrete pain.

---

#### Trade-off 2: Dynamic typing vs. gradual typing as default

| Side | Argument |
|------|----------|
| **Dynamic (current default)** | Faster iteration, less ceremony, easier for new users |
| **Gradual annotations** | Better tooling (completion, inference), catches bugs earlier |

**Current stance**: Dynamic by default. Gradual typing is opt-in via annotations. Static enforcement via `izi check`.

---

#### Trade-off 3: Batteries-included vs. minimal core

| Side | Argument |
|------|----------|
| **Batteries-included (current)** | Easier onboarding, no dependency management needed |
| **Minimal core** | Smaller binary, community-driven stdlib, clear separation |

**Current stance**: Batteries-included for v1.0. Community packages extend beyond stdlib.

---

#### Trade-off 4: Single-threaded async vs. true parallelism

| Side | Argument |
|------|----------|
| **Async/await only** | Simple mental model, no data races, sufficient for I/O-bound workloads |
| **OS threads** | True CPU parallelism, needed for compute-intensive code |

**Current stance**: Async/await for v0.4. Threads deferred to v1.x based on community demand.

---

## Revision History

| Date | Author | Change |
|------|--------|--------|
| 2026-02-21 | IziLang Team | Initial analysis document created from issue #roadmap |

---

**Document maintained by**: IziLang Development Team  
**Feedback**: Open a GitHub issue or discussion  
**Related docs**: [ROADMAP.md](ROADMAP.md) · [DECISIONS.md](DECISIONS.md) · [STATUS.md](STATUS.md)

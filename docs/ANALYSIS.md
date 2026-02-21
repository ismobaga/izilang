# IziLang — Project Analysis, Roadmap & Feature Definition

**Date**: February 2026  
**Version Analysed**: v0.3.0 / v0.4.0-dev  
**Author**: IziLang Development Team

This document provides a full extensive analysis of the IziLang programming language, addressing the long-term vision, core design validation, feature inventory, v1.0 scope, and contribution guidelines.

---

## 1️⃣ Project Identity Analysis

### Name & Philosophy

**IziLang** is named after the word *easy* — the language is designed to be genuinely approachable without sacrificing power or correctness. The core promise is:

> *IziLang should be easy to learn, easy to read, and easy to extend — while being serious enough for real-world programs.*

### Target Audience

| Audience | Primary Use Case |
|----------|-----------------|
| **Students & Beginners** | Learning programming concepts without noise |
| **Scripting Engineers** | Replacing shell scripts with readable code |
| **Prototype Developers** | Rapid iteration without type ceremony |
| **Language Enthusiasts** | Exploring interpreter/compiler implementation |

### Design Pillars

1. **Clarity over cleverness** — Syntax reads like intent
2. **Safety by default** — Clear errors, no undefined behaviour surprises
3. **Progressive complexity** — Simple things are simple; hard things are possible
4. **Tooling first** — A language is only as good as its developer experience
5. **Batteries included** — Rich stdlib so users can focus on problems, not plumbing

### Language Personality

IziLang occupies the space between **Python** (approachability, dynamic typing) and **JavaScript** (functions-first, event model, modules) while learning from **Rust** (error messages, explicit semantics) and **Kotlin** (optional typing, null safety pattern).

---

## 2️⃣ Core Language Audit

### Lexer

| Feature | Status | Notes |
|---------|--------|-------|
| All operators | ✅ | `+`, `-`, `*`, `/`, `!`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `?`, `:`, `->` |
| Keywords | ✅ | `fn`, `var`, `if`, `else`, `while`, `for`, `return`, `break`, `continue`, `class`, `import`, `export`, `match`, `case`, `try`, `catch`, `finally`, `throw`, `this`, `super`, `async`, `await`, `macro` |
| Literals | ✅ | Integers, decimals, strings, booleans, `nil` |
| String interpolation | ✅ | `"Hello ${name}!"` |
| Single-line comments | ✅ | `// comment` |
| Block comments | ✅ | `/* comment */` (with nesting detection) |
| Line/column tracking | ✅ | Used in all error messages |

**Verdict**: ✅ Production-quality lexer. No known issues.

---

### Parser

| Feature | Status | Notes |
|---------|--------|-------|
| Operator precedence | ✅ | 8-level hierarchy, correct |
| All expression types | ✅ | Binary, unary, grouping, call, index, assign, ternary, match, interpolation |
| All statement types | ✅ | var, if, while, for, fn, return, break, continue, import, export, try, class, match |
| Async functions | ✅ | `async fn name(params) { ... }` |
| Await expressions | ✅ | `await someCall()` |
| Macro definitions | ✅ | `macro name(params) { body }` |
| Macro expansion | ✅ | `name!(args)` |
| Error recovery | ⚠️ | Stops at first error; multi-error recovery not implemented |
| Optional semicolons | ✅ | Newline-aware parsing |

**Verdict**: ✅ Robust parser. Error recovery (multi-error reporting) is a known gap for v1.0.

---

### Type System

| Feature | Status | Notes |
|---------|--------|-------|
| Dynamic types | ✅ | `nil`, `bool`, `number`, `string`, arrays, maps, functions |
| Gradual typing | ✅ | Optional annotations: `var x: Number = 5` |
| Function signatures | ✅ | `fn add(a: Number, b: Number): Number { ... }` |
| Runtime type checks | ✅ | Annotations checked at runtime |
| Static type inference | ❌ | Planned post-v1.0 |
| Generics | ❌ | Planned post-v1.0 |
| Algebraic data types | ❌ | Long-term consideration |

**Verdict**: ✅ Gradual typing is the right approach for IziLang's audience. Static inference can come later.

---

### Runtime & Execution

| Feature | Status | Notes |
|---------|--------|-------|
| Tree-walking interpreter | ✅ | Default, stable, production-ready |
| Bytecode compiler | ✅ | Full language support |
| Bytecode VM | ⚠️ | Functional (`--vm` flag); some edge cases in class/async |
| Mark-and-Sweep GC | ✅ | Implemented in v0.3.0 |
| Reference counting | ✅ | Used for owned values |
| Stack overflow protection | ✅ | Detects and reports infinite recursion |
| Closures | ✅ | Lexical scoping, captured variables |
| First-class functions | ✅ | Functions as values, higher-order patterns |
| Tail-call optimisation | ❌ | Not implemented |

---

### Error Handling

| Feature | Status | Notes |
|---------|--------|-------|
| `try / catch / finally` | ✅ | Full exception semantics |
| `throw` statement | ✅ | Throw any value |
| Stack traces | ⚠️ | Function names shown; source file sometimes missing |
| Typed exceptions | ⚠️ | No structured error types yet |
| Source-location errors | ✅ | Line, column, code snippet, caret |
| Color-coded output | ✅ | Terminal colour support |

---

### Standard Library

| Module | Status | Key Functions |
|--------|--------|---------------|
| `std.math` | ✅ | `sqrt`, `pow`, `sin`, `cos`, `floor`, `ceil`, `round`, `abs`, `min`, `max`, `PI`, `E` |
| `std.string` | ✅ | `substring`, `split`, `toUpper`, `toLower`, `trim`, `replace`, `indexOf`, `startsWith`, `endsWith`, `join` |
| `std.array` | ✅ | `map`, `filter`, `reduce`, `sort`, `reverse`, `concat`, `slice` |
| `std.io` | ✅ | `readFile`, `writeFile`, `appendFile`, `fileExists` |
| `std.json` | ✅ | `parse`, `stringify` |
| `std.time` | ✅ | `now`, `sleep`, `format` |
| `std.regex` | ✅ | `match`, `replace`, `test` |
| `std.http` | ✅ | `get`, `post`, `request` |
| `std.net` | ✅ | TCP/UDP socket primitives |
| `std.log` | ✅ | `info`, `warn`, `error`, `debug` |
| `std.ipc` | ✅ | Named-pipe IPC |
| `std.assert` | ✅ | `ok`, `equals`, `throws` |
| `std.process` | ✅ | `env`, `args`, `exit` |
| `std.fs` | ✅ | Extended filesystem operations |
| `std.async` | ⚠️ | Skeleton; full event-loop I/O in progress |

---

## 3️⃣ Feature Inventory (Current vs Missing)

### Existing / Implemented ✅

```
Core Language
  ✅ Variables (var)
  ✅ All primitive types (nil, bool, number, string)
  ✅ Arrays and Maps (collections)
  ✅ Control flow (if/else, while, for)
  ✅ Functions (fn, closures, first-class)
  ✅ Classes and OOP (class, inheritance, this, super)
  ✅ Pattern matching (match/case with guards)
  ✅ Exception handling (try/catch/finally/throw)
  ✅ Module system (import/export, named and wildcard)
  ✅ String interpolation ("Hello ${name}")
  ✅ Ternary operator (condition ? a : b)
  ✅ Gradual typing (optional annotations)
  ✅ Async/await (async fn, await)
  ✅ Macros (macro name(x) { body }, name!(args))
  ✅ Break/continue statements
  ✅ Optional semicolons

Runtime
  ✅ Tree-walking interpreter
  ✅ Bytecode compiler + VM
  ✅ Mark-and-Sweep garbage collector
  ✅ Reference counting (shared_ptr)
  ✅ Stack overflow protection
  ✅ Closures and lexical scoping
  ✅ Native function interface

Tooling
  ✅ Full CLI (run, repl, fmt, check, test, bench, compile, chunk)
  ✅ Interactive REPL
  ✅ Code formatter (izi fmt)
  ✅ Semantic analyser (izi check)
  ✅ Benchmark runner (izi bench)
  ✅ Native compilation (izi compile)
  ✅ AST optimiser (constant folding, DCE)

Standard Library
  ✅ 14+ native modules (math, string, array, io, json, time, regex, http, net, log, ipc, assert, process, fs)
  ✅ 100+ native functions

Testing & CI
  ✅ 185 test cases, 1,087 assertions, 100% passing
  ✅ Multi-platform CI (Linux, macOS, Windows)
  ✅ Automated release builds
```

### Missing / To Evaluate ❌

```
Language Features
  ❌ Destructuring assignment (var {a, b} = obj or var [x, y] = arr)
  ❌ Spread/rest operator (fn(a, ...rest) and [...arr, 4])
  ❌ Optional chaining (obj?.field)
  ❌ Nullish coalescing (x ?? default)
  ❌ Generator functions (yield)
  ❌ Decorators / annotations (@decorator)
  ❌ String multi-line literals
  ❌ Bitwise operators (&, |, ^, ~, <<, >>)
  ❌ Integer vs float distinction (all numbers are double)

Type System
  ❌ Static type inference (infer from assignment)
  ❌ Generics / templates
  ❌ Union types (Number | String)
  ❌ Nullable types / Option<T>
  ❌ Interface / trait system

Runtime
  ❌ Full async event loop I/O (std.async)
  ❌ Promise chaining (then/catch/finally)
  ❌ Parallel collections (thread-safe arrays/maps)
  ❌ Tail-call optimisation
  ❌ JIT compilation (post-v1.0)

Tooling
  ❌ LSP diagnostics (connected to actual compiler)
  ❌ LSP auto-completion
  ❌ LSP hover / go-to-definition
  ❌ VS Code extension (full, published)
  ❌ Debug Adapter Protocol (DAP)
  ❌ Package manager backend
  ❌ Package registry

Standard Library
  ❌ std.async (full implementation)
  ❌ std.crypto (hashing, encryption)
  ❌ std.xml / std.csv (data formats)
  ❌ std.test (built-in test framework for .iz files)
  ❌ std.ui improvements (cross-platform UI)
```

---

## 4️⃣ v1.0 Scope Definition (MVP)

v1.0 represents **language stability with backward compatibility guarantees**. The following criteria must be met before tagging v1.0:

### Required for v1.0

#### Language
- [x] Complete core syntax (variables, functions, control flow, OOP, closures)
- [x] Pattern matching
- [x] Exception handling
- [x] Module system (import/export)
- [x] String interpolation
- [x] Gradual typing (optional annotations)
- [x] Async/await primitives
- [ ] Full async I/O (`std.async` event loop)
- [ ] Destructuring assignment (high-value, ergonomic)
- [ ] Optional chaining `obj?.field` (high-value, safe access)
- [ ] Nullish coalescing `x ?? default` (high-value, null safety)

#### Runtime
- [x] Stable tree-walking interpreter
- [x] Bytecode compiler
- [x] Mark-and-Sweep GC
- [ ] VM production-ready (no known crashes)

#### Tooling
- [x] Full CLI toolchain
- [x] Code formatter (`izi fmt`)
- [x] Semantic analyser (`izi check`)
- [ ] LSP server with diagnostics and autocompletion
- [ ] VS Code extension published on marketplace
- [ ] Package manager MVP (`izi-pkg init/install/publish`)
- [ ] Debug Adapter Protocol (DAP) support

#### Standard Library
- [x] Core modules (math, string, array, io, json, time, regex, http)
- [ ] `std.async` stable API
- [ ] `std.test` — built-in test framework for `.iz` files

#### Documentation
- [x] Comprehensive user guide
- [x] CLI reference
- [x] Standard library reference
- [x] Architecture documentation
- [ ] Interactive tutorial ("Learn IziLang in Y minutes")
- [ ] Formal language grammar (EBNF)

#### Stability
- [ ] Syntax freeze — no breaking changes after v1.0
- [ ] API freeze — stdlib API locked
- [ ] SemVer adopted
- [ ] Deprecation policy published

### Not Required for v1.0

- Static type inference (can ship as v1.1 or v2.0)
- Generics (post-v1.0)
- JIT compilation (post-v1.0)
- WASM target (post-v1.0)
- C FFI (post-v1.0)
- Parallel collections (post-v1.0)

---

## 5️⃣ Roadmap Proposal

### Foundation (v0.4.0 — Q3 2026)

These items complete the **developer experience** necessary before ecosystem growth.

| Feature | Priority | Effort | Status |
|---------|----------|--------|--------|
| Code formatter (`izi fmt`) | 🔴 Critical | Low | ✅ Done |
| Async/await syntax & runtime | 🔴 Critical | Medium | ✅ Done |
| Macro system | 🔴 Critical | Medium | ✅ Done |
| LSP server — diagnostics | 🔴 Critical | High | 🟡 In Progress |
| LSP server — autocompletion | 🔴 Critical | High | ❌ Not started |
| Full async I/O (`std.async`) | 🔴 Critical | High | 🟡 In Progress |
| VM production-ready | 🟠 High | Medium | 🟡 In Progress |

### Productivity (v0.5.0 — Q4 2026)

These items improve **day-to-day developer ergonomics**.

| Feature | Priority | Effort | Status |
|---------|----------|--------|--------|
| Destructuring assignment | 🟠 High | Medium | ❌ Not started |
| Optional chaining (`?.`) | 🟠 High | Low | ❌ Not started |
| Nullish coalescing (`??`) | 🟠 High | Low | ❌ Not started |
| Spread/rest operator | 🟠 High | Medium | ❌ Not started |
| VS Code extension (full) | 🟠 High | Medium | ❌ Not started |
| Debug Adapter Protocol (DAP) | 🟠 High | High | ❌ Not started |
| `std.test` built-in framework | 🟠 High | Medium | ❌ Not started |
| Package manager backend | 🟠 High | High | ❌ Not started |

### Ecosystem (v1.0.0 — Q1 2027)

These items enable **community and adoption**.

| Feature | Priority | Effort | Status |
|---------|----------|--------|--------|
| Package registry (public) | 🟠 High | High | ❌ Not started |
| Language freeze (syntax/API) | 🔴 Critical | Low | ❌ Not started |
| SemVer + deprecation policy | 🔴 Critical | Low | ❌ Not started |
| Interactive tutorial | 🟡 Medium | Medium | ❌ Not started |
| Formal grammar (EBNF) | 🟡 Medium | Low | ❌ Not started |
| Contributor governance | 🟡 Medium | Low | ❌ Not started |
| 50+ packages in registry | 🟡 Medium | Community | ❌ Not started |

### Advanced (Post-v1.0)

These are **future directions** not committed to any specific version.

| Feature | Notes |
|---------|-------|
| Static type inference | TypeScript-style, opt-in |
| Generics / templates | `fn identity<T>(x: T): T` |
| Union types | `Number \| String` |
| JIT compilation | LLVM or Cranelift backend |
| WASM target | Run IziLang in the browser |
| C FFI | Call native libraries |
| Python interop | `import` from Python packages |
| Actor model | Isolated processes, message passing |
| Macros expansion (hygienic) | Compile-time code generation |
| Reflection API | Inspect types at runtime |

---

## 6️⃣ Feature Suggestion Guidelines

### How to Propose a Feature

All feature suggestions are welcome! Use the following process to submit a well-formed proposal.

#### Step 1: Check Existing Work

Before opening an issue, check:
- [ ] The [ROADMAP.md](ROADMAP.md) to see if it's already planned
- [ ] Open and closed GitHub issues for prior discussion
- [ ] The [V03_SPECIFICATION.md](V03_SPECIFICATION.md) to understand current behaviour

#### Step 2: Open a GitHub Issue

Use the **Feature Request** template and fill in:

1. **Title**: Clear and specific (e.g. "Add optional chaining operator `?.`")

2. **Use Case**: One real scenario where this feature helps
   ```
   As a developer parsing JSON APIs, I often access nested fields that may be nil.
   Without optional chaining I write: `if obj != nil and obj.user != nil ...`
   With optional chaining: `print(obj?.user?.name)`
   ```

3. **Proposed Syntax**: Show what the feature looks like
   ```izi
   var name = user?.profile?.name ?? "Anonymous";
   ```

4. **Semantics**: Explain the exact behaviour
   - `obj?.field` returns `nil` if `obj` is `nil`, otherwise `obj.field`
   - Short-circuits: if `obj` is nil the right side is never evaluated

5. **Alternatives Considered**: What other ways could this be expressed?

6. **Breaking Changes**: Does this change existing behaviour?

7. **Implementation Complexity**: Is this a lexer change, parser change, or runtime change?

#### Step 3: Community Discussion

Label the issue `feature-request`. The community votes with 👍/👎. Core team reviews monthly.

### Prioritisation Criteria

Features are evaluated on four axes:

| Axis | Weight | Description |
|------|--------|-------------|
| **Impact** | 40% | How many users benefit? (ergonomics wins here) |
| **Alignment** | 30% | Does it fit IziLang's philosophy? |
| **Effort** | 20% | Implementation complexity and risk |
| **Urgency** | 10% | Is it blocking adoption or a common workaround? |

### Feature Categories & Labels

| Label | Meaning |
|-------|---------|
| `language-feature` | New syntax or semantics |
| `stdlib` | New standard library function or module |
| `tooling` | CLI, LSP, formatter, debugger |
| `performance` | Runtime or compiler optimisation |
| `developer-experience` | Error messages, diagnostics, REPL |
| `documentation` | Guides, tutorials, API reference |
| `good-first-issue` | Suitable for new contributors |
| `help-wanted` | Core team needs assistance |

### What Makes a Good Feature for IziLang

✅ **Good fit**:
- Reduces boilerplate for common patterns
- Makes the language safer (avoids common runtime errors)
- Matches what JS/Python/Kotlin developers already know
- Has a clear and unambiguous syntax
- Does not add runtime overhead by default

❌ **Poor fit**:
- Requires memorising complex rules
- Duplicates existing constructs without clear advantage
- Adds significant implementation burden for niche use cases
- Breaks the simplicity promise

### Examples of Well-Formed Feature Requests

**Good**: "Add `??` nullish coalescing"
- Common pattern: `var x = value != nil ? value : default;`
- Proposed: `var x = value ?? default;`
- Non-breaking addition to the language
- Widely understood from JS/Swift/Kotlin

**Good**: "Add `?.` optional chaining"
- Eliminates nested nil checks
- Maps to existing null-safety patterns
- Pure syntactic sugar over existing semantics

**Weak**: "Add a type system like Haskell"
- Too broad and complex
- Does not fit "easy" positioning
- Should be broken into smaller, concrete proposals

---

## 7️⃣ Open Questions

The following questions are actively discussed and not yet resolved. Contributions to these discussions are especially welcome.

### Q1: Should `izi` ship a built-in test runner for `.iz` files?

**Context**: Currently, tests are C++ (Catch2). Users writing IziLang programs have no native way to write unit tests in IziLang itself.

**Options**:
- A) Add `std.test` module with `describe/it/expect` conventions
- B) Add `izi test` that discovers `*_test.iz` files and runs them
- C) Both A and B
- D) Document how to use IziLang's exception system for assertions (no new infrastructure)

**Recommendation**: Option C — adds the most value for real-world IziLang users.

---

### Q2: Should integer and float be separate types?

**Context**: All numbers are currently `double` (IEEE 754 64-bit float). This avoids complexity but loses precision for large integers.

**Options**:
- A) Keep unified `Number` type (current)
- B) Add `Int` and `Float` as separate types; `Number` becomes a union
- C) Add `BigInt` for arbitrary precision alongside `Number`

**Tradeoffs**:
- Option A: Simple, predictable, no surprises for new users
- Option B: Familiar to typed-language users; adds runtime overhead
- Option C: Solves precision without breaking compatibility

**Recommendation**: Keep Option A for v1.0. Revisit with static typing in post-v1.0.

---

### Q3: What is the async execution model?

**Context**: `async`/`await` syntax is implemented. The execution semantics need formalisation.

**Options**:
- A) Cooperative multitasking (green threads / fibers)
- B) OS thread per async task
- C) Single-threaded event loop (Node.js model)
- D) Coroutines as a library primitive

**Recommendation**: Option C — single-threaded event loop. Well-understood model, low implementation risk, fits scripting use case.

---

### Q4: Should optional chaining (`?.`) and nullish coalescing (`??`) be added before v1.0?

**Context**: These operators are among the most requested features in similar languages and address a very common pain point (nil/null safety).

**Verdict**: ✅ **Yes** — both operators should be added in v0.5.0 as part of the "Productivity" milestone. They are:
- Non-breaking additions
- Low implementation complexity (lexer + parser + interpreter changes only)
- High daily-use value

---

### Q5: How should the package manager interact with the module system?

**Context**: The module system uses file-based paths and native module names. A package manager needs to map package names to versioned directories.

**Proposal**:
```izi
// Current (local)
import { parse } from "json";

// With package manager
import { validate } from "izi-json-schema@1.2.0";
// Resolved from ~/.izi/packages/izi-json-schema/1.2.0/index.iz
```

**Open items**:
- [ ] Registry URL format
- [ ] Lockfile format (`izi.lock`)
- [ ] Conflict resolution strategy
- [ ] Private registry support

---

### Q6: Should IziLang support WASM as a compilation target?

**Context**: A WASM target would let IziLang programs run in browsers without a server.

**Verdict**: Post-v1.0. Requires significant VM refactoring. Low priority until the language is stable.

---

## Conclusion

IziLang is a **well-architected, feature-rich, and actively developed language** that has already delivered on most of its v0.3.0 promises. The roadmap is realistic and the remaining work (LSP, package manager, async I/O, productivity operators) is well-scoped.

The path to v1.0 is clear:

1. **v0.4.0** (Q3 2026): Complete async I/O, LSP MVP, VM stability
2. **v0.5.0** (Q4 2026): Productivity features (destructuring, `?.`, `??`), package manager, DAP
3. **v1.0.0** (Q1 2027): Language freeze, ecosystem launch, community governance

IziLang's name carries a promise — and the current implementation honours it.

---

**Document maintained by**: IziLang Development Team  
**Feedback**: Open a GitHub issue or discussion  
**Related docs**: [ROADMAP.md](ROADMAP.md) · [DECISIONS.md](DECISIONS.md) · [STATUS.md](STATUS.md)

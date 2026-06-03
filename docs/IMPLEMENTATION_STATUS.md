# IziLang Implementation Status

This document is the source of truth for the implementation status of all major language features and tooling. It is updated as features are completed, changed, or deprecated.

**Legend:**
- ✅ Complete — stable and tested
- 🔶 Partial — syntax or partial runtime exists; full support incomplete
- 🧪 Experimental — implemented but not stable/production-ready
- 🚧 Planned — on the roadmap, not yet started or very early
- ❌ Not implemented — no implementation exists

---

## Core Language Features

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| Variables (`var`) | ✅ Complete | Yes | Yes | Yes | Stable |
| Functions (`fn`) | ✅ Complete | Yes | Yes | Yes | Stable, including closures |
| First-class functions | ✅ Complete | Yes | Yes | Yes | Closures, higher-order functions |
| Control flow (`if`/`else`/`while`/`for`) | ✅ Complete | Yes | Yes | Yes | Stable |
| `break` / `continue` | ✅ Complete | Yes | Yes | Yes | Stable |
| `return` | ✅ Complete | Yes | Yes | Yes | Stable |
| Arithmetic & comparison operators | ✅ Complete | Yes | Yes | Yes | Stable |
| String interpolation | ✅ Complete | Yes | Yes | Yes | Template literal syntax |
| Nullish coalescing (`??`) | ✅ Complete | Yes | Yes | Yes | Stable |
| `nil` value | ✅ Complete | Yes | Yes | Yes | Stable |
| Boolean literals (`true`/`false`) | ✅ Complete | Yes | Yes | Yes | Stable |

---

## Collections

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| Arrays | ✅ Complete | Yes | Yes | Yes | Rich operations (push, pop, slice, etc.) |
| Maps (objects) | ✅ Complete | Yes | Yes | Yes | Key-value store |
| Sets | ✅ Complete | Yes | Partial | Yes | Via stdlib |

---

## OOP & Classes

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| Classes (`class`) | ✅ Complete | Yes | Yes | Yes | Stable |
| Inheritance (`extends`) | ✅ Complete | Yes | Yes | Yes | Single inheritance |
| `this` binding | ✅ Complete | Yes | Yes | Yes | Stable |
| `super` calls | ✅ Complete | Yes | Yes | Yes | Stable |
| Constructors | ✅ Complete | Yes | Yes | Yes | Stable |

---

## Type System

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| Gradual typing (optional annotations) | ✅ Complete | Yes | Partial | Yes | Annotations accepted; runtime checks limited |
| Semantic analysis (`izi check`) | ✅ Complete | Yes | N/A | Yes | Unused vars, dead code detection |

---

## Error Handling

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| `try`/`catch`/`finally` | ✅ Complete | Yes | Yes | Yes | Stable |
| `throw` | ✅ Complete | Yes | Yes | Yes | Stable |
| Rich error messages | ✅ Complete | Yes | Yes | Yes | Line/column, visual indicators |
| Stack traces | ✅ Complete | Yes | Partial | Yes | Full in interpreter |

---

## Module System

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| `import` / `export` | ✅ Complete | Yes | Yes | Yes | Stable |
| Relative imports | ✅ Complete | Yes | Yes | Yes | Stable |
| Stdlib modules | 🔶 Partial | Yes | Partial | Partial | See stdlib table below |

---

## Pattern Matching

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| `match` expressions | ✅ Complete | Yes | Yes | Yes | Stable |
| Tuple/destructuring patterns | ✅ Complete | Yes | Yes | Yes | Stable |
| Wildcard patterns (`_`) | ✅ Complete | Yes | Yes | Yes | Stable |

---

## Async / Concurrency

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| `async fn` syntax | 🔶 Partial | Parse only | Parse only | Needs audit | Runtime incomplete |
| `await` syntax | 🔶 Partial | Parse only | Parse only | Needs audit | Runtime incomplete |
| `std.async` event loop | 🚧 Planned | No | No | No | v0.4 target |

---

## Macros

| Feature | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| Macro definitions (`macro name!(args)`) | 🧪 Experimental | Yes | Partial | Partial | Expand at call site; limitations apply |
| Macro expansion | 🧪 Experimental | Yes | Partial | Partial | Not production-ready |

---

## Standard Library

| Module | Status | Interpreter | VM | Tests | Notes |
|---|---|---|---|---|---|
| `std.math` | ✅ Complete | Yes | Yes | Yes | Stable |
| `std.string` | ✅ Complete | Yes | Yes | Yes | Stable |
| `std.array` | ✅ Complete | Yes | Yes | Yes | Stable |
| `std.io` | 🔶 Partial | Yes | Partial | Partial | Basic file I/O |
| `std.env` | 🔶 Partial | Yes | Partial | Partial | Environment variables |
| `std.fs` | 🔶 Partial | Yes | Partial | Partial | Filesystem operations |
| `std.json` | 🧪 Experimental | Yes | Partial | Partial | Basic parse/stringify |
| `std.time` | 🧪 Experimental | Yes | Partial | Partial | Basic time functions |
| `std.regex` | 🧪 Experimental | Yes | Partial | Partial | Basic regex matching |
| `std.http` | 🧪 Experimental | Yes | Partial | No | HTTP client; limited |
| `std.net` | 🧪 Experimental | Yes | Partial | No | TCP socket primitives |
| `std.log` | 🧪 Experimental | Yes | Partial | No | Logging helpers |
| `std.ipc` | 🧪 Experimental | Yes | Partial | Partial | Named-pipe IPC |
| `std.assert` | ✅ Complete | Yes | Yes | Yes | Assertion helpers |
| `std.process` | 🔶 Partial | Yes | Partial | Partial | Process spawn/exit |
| `std.audio` | 🚧 Planned | No | No | No | Planned with raylib |
| `std.image` | 🚧 Planned | No | No | No | Planned with raylib |
| `std.ui` | 🚧 Planned | No | No | No | Planned with raylib |
| `std.async` | 🚧 Planned | No | No | No | Event loop; v0.4 target |

---

## Tooling (CLI)

| Tool / Command | Status | Notes |
|---|---|---|
| `izi run` | ✅ Complete | Execute .iz files |
| `izi build` | ✅ Complete | Compile/check without running |
| `izi check` | ✅ Complete | Syntax + semantic analysis |
| `izi repl` | ✅ Complete | Interactive REPL; readline is optional |
| `izi test` | ✅ Complete | Recursive test discovery in `tests/`; `--examples` flag |
| `izi fmt` | 🧪 Experimental | Formatter; config via `.izifmt.toml` |
| `izi bench` | ✅ Complete | Performance benchmarking |
| `izi compile` | 🧪 Experimental | Native compilation via clang/gcc; limited |
| `izi chunk` | 🔶 Partial | Bytecode serialization; VM load path works |
| `izi doctor` | ✅ Complete | Local setup diagnostics |
| `izi version` | ✅ Complete | Show version |
| `izi help` | ✅ Complete | Help for all commands |

---

## Execution Engines

| Engine | Status | Notes |
|---|---|---|
| Tree-walker interpreter | ✅ Complete | Default; full feature coverage |
| Bytecode VM | 🔶 Partial | Core operations work; some features incomplete (see `docs/VM_PARITY_CHECKLIST.md`) |
| Native compilation | 🧪 Experimental | Generates C++ or compiles via clang; limited stdlib |

---

## Developer Tooling / Ecosystem

| Tool | Status | Notes |
|---|---|---|
| GitHub Actions CI | ✅ Complete | Build + C++ tests + `izi test` on Ubuntu/macOS/Windows |
| readline REPL | 🔶 Partial | Opt-in at build time (`--readline`); fallback available |
| LSP server | 🚧 Planned | In `tools/lsp/`; not yet functional |
| Package manager (`izi-pkg`) | 🚧 Planned | In `tools/pkg/`; not yet implemented |
| VS Code extension | 🚧 Planned | In `tools/vscode-extension/`; syntax highlighting only |
| Formatter config (`.izifmt.toml`) | 🧪 Experimental | TOML config supported; formatter itself is experimental |

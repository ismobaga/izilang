# IziLang Design Decisions

**Last Updated**: February 11, 2026  
**Status**: Living document (frozen sections marked 🔒)

---

## Purpose

This document records **explicit design decisions** made for the IziLang language. Each decision includes:

- **Rationale** - Why this choice was made
- **Alternatives** - What other options were considered
- **Implications** - How this affects the language
- **Status** - Frozen (🔒) or open to revision (🔓)

Once a decision is marked **frozen (🔒)**, it cannot be changed without a major version bump.

---

## Table of Contents

1. [Core Language Philosophy](#1-core-language-philosophy)
2. [Execution Model](#2-execution-model)
3. [Type System](#3-type-system)
4. [Memory Management](#4-memory-management)
5. [Error Handling](#5-error-handling)
6. [Concurrency Model](#6-concurrency-model)
7. [Module System](#7-module-system)
8. [Standard Library](#8-standard-library)
9. [Syntax Choices](#9-syntax-choices)
10. [Performance vs. Simplicity](#10-performance-vs-simplicity)
11. [Backward Compatibility](#11-backward-compatibility)

---

## 1. Core Language Philosophy

### 1.1 Target Audience
**Status**: 🔒 **FROZEN**

**Decision**: IziLang targets **intermediate programmers** who want expressiveness without complexity.

**Rationale**:
- Beginners benefit from clear syntax and helpful errors
- Experts appreciate powerful features (closures, pattern matching)
- Not trying to replace systems languages (C/C++/Rust)
- Not competing with highly specialized DSLs

**Implications**:
- Syntax should be familiar (C-like, not Haskell-like)
- Error messages must be educational
- Standard library should be practical, not minimal
- Performance is important but not at the cost of simplicity

---

### 1.2 Language Paradigm
**Status**: 🔒 **FROZEN**

**Decision**: **Multi-paradigm** with focus on **imperative + functional** styles.

**Supported Paradigms**:
- ✅ Imperative (variables, loops, statements)
- ✅ Functional (first-class functions, closures, higher-order functions)
- ✅ Procedural (functions, modules)
- 🔓 Object-Oriented (planned for v0.2, class-based)

**NOT Supported**:
- ❌ Pure functional (no immutability enforcement)
- ❌ Logic programming (no Prolog-style)
- ❌ Reactive programming (no built-in observables)

**Rationale**: Most real-world programs benefit from mixing paradigms.

---

## 2. Execution Model

### 2.1 Dual Execution Modes
**Status**: 🔓 **OPEN** (may consolidate to single mode)

**Decision**: Support **both tree-walking interpreter and bytecode VM**.

#### Tree-Walking Interpreter
- Direct AST traversal
- Slower but easier to debug
- Default mode for development

#### Bytecode Virtual Machine
- Stack-based architecture
- Faster execution (in theory)
- Production mode

**Rationale**:
- Interpreter allows rapid development and clear errors
- VM provides performance when needed
- Similar approach to Python (CPython has both modes conceptually)

**Alternatives Considered**:
1. **Interpreter only** - Simple but slow for production
2. **VM only** - Faster but harder to debug
3. **JIT compilation** - Too complex for v0.1

**Implications**:
- More code to maintain (two execution paths)
- VM currently has bugs (needs fixing)
- May consolidate in future if VM proves unreliable

**Current Status**: Interpreter is default; VM is experimental.

---

### 2.2 Compilation vs. Interpretation
**Status**: 🔒 **FROZEN**

**Decision**: IziLang is an **interpreted language** (not compiled to native).

**Rationale**:
- Faster development iteration
- Simpler distribution (no compiler toolchain needed)
- Cross-platform by default
- Can add native compilation later (v0.3+)

**Alternatives**:
1. **Ahead-of-time (AOT) compilation** - Would require complex toolchain
2. **WASM target** - Possible future addition
3. **JIT compilation** - Planned for v1.0+

**Implications**:
- Slower startup time (parse + execute)
- No separate compilation step
- Distributes source code (or bytecode)

---

## 3. Type System

### 3.1 Static vs. Dynamic Typing
**Status**: 🔒 **FROZEN** (for v0.1)

**Decision**: IziLang is **dynamically typed** with runtime type checks.

**Rationale**:
- Easier to learn (no type annotations required)
- Faster prototyping
- Similar to Python, JavaScript, Ruby
- Can add gradual typing later (v0.2+)

**Alternatives**:
1. **Static typing** - Would require type inference engine
2. **Gradual typing** - Planned for v0.2 (optional annotations)

**Example**:
```izi
var x = 42;          // Type inferred at runtime (number)
x = "hello";         // Allowed! Type changes at runtime
```

**Implications**:
- Runtime type errors possible
- No compile-time type checking
- Slower execution (type checks at runtime)

---

### 3.2 Type Coercion
**Status**: 🔒 **FROZEN**

**Decision**: **No implicit type coercion** (except for truthiness).

**Rationale**:
- Explicit is better than implicit (Python philosophy)
- Avoids JavaScript-style confusion (`"5" + 3 === "53"`)
- Errors caught early

**Rules**:
```izi
// ✅ Allowed (explicit)
var x = string(42);        // "42"
var y = number("3.14");    // 3.14

// ❌ Forbidden (implicit)
var z = "5" + 3;           // Runtime error: Cannot add string and number

// ✅ Exception: Truthiness
if ("hello") { ... }       // Non-empty string is truthy
```

**Truthiness Rules** (frozen):
- `nil` → false
- `false` → false
- `0` → true (unlike JavaScript!)
- `""` (empty string) → false
- Empty array `[]` → false
- Empty map `{}` → false
- Everything else → true

**Implications**:
- More runtime errors initially
- Clearer semantics (no surprises)
- Easier to reason about code

---

### 3.3 Primitive Types
**Status**: 🔒 **FROZEN**

**Decision**: Four primitive types + two collection types.

#### Primitives
1. **nil** - Absence of value (like `null` in JavaScript)
2. **bool** - `true` or `false`
3. **number** - 64-bit floating point (IEEE 754 double)
4. **string** - UTF-8 encoded text

#### Collections
5. **array** - Ordered list, 0-indexed
6. **map** - Key-value store (hash table)

**Rationale**:
- Numbers as doubles avoids integer overflow surprises
- Single number type simplifies the language
- No separate integer type (like Lua, JavaScript)

**Implications**:
- Integer precision limited to 53 bits (2^53 - 1)
- No bigint support (yet)
- Fast numeric operations

**Future Additions** (v1.0+):
- 🔓 **bigint** - Arbitrary precision integers
- 🔓 **regex** - First-class regular expressions
- 🔓 **bytes** - Byte arrays for binary data

---

## 4. Memory Management

### 4.1 Garbage Collection Strategy
**Status**: 🔓 **OPEN** (critical decision needed)

**Current State**: Using `std::shared_ptr` (reference counting).

**Decision Needed**: Choose explicit GC algorithm.

#### Option 1: Reference Counting (Current)
**Pros**:
- Simple to implement (already done)
- Predictable deallocation
- Low latency (no stop-the-world pauses)

**Cons**:
- Cannot handle circular references
- Overhead on every assignment
- Slower than tracing GC for some workloads

#### Option 2: Mark-and-Sweep GC
**Pros**:
- Handles circular references
- Industry standard (used by Python, Ruby, Go)
- Can be generational

**Cons**:
- Stop-the-world pauses
- More complex implementation

#### Option 3: Hybrid (Reference Counting + Cycle Detection)
**Pros**:
- Best of both worlds
- Used by Swift

**Cons**:
- Most complex
- Two GC algorithms to maintain

**Recommendation**: **Mark-and-Sweep** with generational extension (Option 2).

**Timeline**: Decision by end of Q1 2026, implementation in Q3 2026.

---

### 4.2 Memory Safety
**Status**: 🔒 **FROZEN**

**Decision**: IziLang is **memory-safe** (no manual memory management).

**Guarantees**:
- No `malloc`/`free` or `new`/`delete`
- No pointer arithmetic
- No use-after-free bugs
- No double-free bugs

**Rationale**:
- Safety over performance
- Focus on correctness
- Easier to learn

**Implications**:
- Cannot write low-level systems code
- Slightly slower than manual management
- GC overhead

---

## 5. Error Handling

### 5.1 Exception Model
**Status**: 🔒 **FROZEN**

**Decision**: Use **exceptions** with `try`/`catch`/`finally` (not `Result` types).

**Syntax**:
```izi
try {
    var data = readFile("config.json");
    process(data);
} catch (err) {
    print("Error: " + err.message);
} finally {
    cleanup();
}
```

**Rationale**:
- Familiar to Java/JavaScript/Python developers
- Separates happy path from error handling
- Explicit (caller must handle or propagate)

**Alternatives**:
1. **Result types** (Rust/Go style) - More explicit but verbose
2. **Error codes** (C style) - Easy to ignore
3. **Conditions** (Common Lisp style) - Too advanced

**Implications**:
- Uncaught exceptions terminate the program
- Stack traces included in exceptions
- Performance cost for exception handling

---

### 5.2 Error Messages
**Status**: 🔒 **FROZEN** (quality standard)

**Decision**: All errors must include:
1. Error type (parse, runtime, etc.)
2. Line and column number
3. Code snippet with caret pointing to error
4. Clear, actionable message

**Example**:
```
Runtime Error at line 3, column 18:
  3 | var result = name + age;
    |                  ^
Cannot add string and number. Operands must be two numbers or two strings.
```

**Rationale**:
- Rust-quality error messages
- Helps beginners learn
- Faster debugging

**Implications**:
- More work to implement errors
- Parser must track source locations
- Worth the effort

---

### 5.3 Stack Traces
**Status**: 🔒 **FROZEN**

**Decision**: All exceptions include **stack traces** with function names.

**Example**:
```
RuntimeError: Division by zero
  at divide() [line 8]
  at calculate() [line 15]
  at main() [line 23]
```

**Future Enhancement** (v0.2):
- Include source file names
- Include line numbers in trace
- Color-coded output

---

## 6. Concurrency Model

### 6.1 Threading vs. Async
**Status**: 🔓 **OPEN** (decision needed for v0.3)

**Decision Needed**: Choose concurrency model.

#### Option 1: No Concurrency (Current)
**Pros**: Simple, no race conditions  
**Cons**: Cannot utilize multiple cores

#### Option 2: OS Threads
**Pros**: True parallelism, familiar  
**Cons**: Heavyweight, requires mutexes

#### Option 3: Async/Await (Recommended)
**Pros**: Lightweight, non-blocking I/O  
**Cons**: Not truly parallel, requires event loop

**Syntax (Proposed)**:
```izi
async fn fetchData(url) {
    var response = await http.get(url);
    return response.json();
}

var data = await fetchData("https://api.example.com");
```

**Recommendation**: **Async/Await** (Option 3) for v0.3.

**Rationale**:
- Most I/O-bound programs don't need true parallelism
- Async/await is familiar (JavaScript, Python, Rust)
- Can add threads later if needed

**Timeline**: Decision by Q2 2026, implementation in Q3 2026.

---

### 6.2 Shared State
**Status**: 🔓 **OPEN**

**Question**: How to handle shared mutable state in concurrent code?

#### Options
1. **No sharing** - Message passing only (Go channels, Erlang actors)
2. **Explicit locks** - Mutexes, semaphores (Java, C++)
3. **Immutable by default** - Functional approach (Clojure)
4. **Software Transactional Memory** (Haskell)

**Recommendation**: Defer decision until concurrency model chosen.

---

## 7. Module System

### 7.1 Import Syntax
**Status**: 🔒 **FROZEN**

**Decision**: ES6-style imports with `import`/`export`.

**Syntax**:
```izi
// Named imports
import { sqrt, pow } from "std.math";

// Wildcard imports (⚠️ implementation incomplete)
import * from "std.string";

// Export
export fn myFunction() { ... }
export var myVariable = 42;
```

**Rationale**:
- Familiar to JavaScript/TypeScript developers
- Explicit (shows what's imported)
- Tree-shakeable (unused exports can be removed)

**Alternatives**:
1. **Python-style** (`from math import sqrt`) - Too similar to JS
2. **Rust-style** (`use std::math::sqrt`) - More verbose
3. **Go-style** (`import "math"`) - Less explicit

---

### 7.2 Module Resolution
**Status**: 🔒 **FROZEN**

**Decision**: Node.js-style module resolution.

**Search Order**:
1. Built-in modules (std.*)
2. Relative paths (./module, ../module)
3. node_modules/ equivalent (future: izi_modules/)
4. Global modules (future: system-wide installation)

**Example**:
```izi
import { readFile } from "std.io";        // Built-in
import { helpers } from "./utils.iz";    // Relative
import { express } from "express";        // Package (future)
```

**Implications**:
- Simple for beginners
- Familiar to JavaScript developers
- Requires package manager (v0.2+)

---

### 7.3 Circular Dependencies
**Status**: 🔒 **FROZEN**

**Decision**: Circular imports are **allowed** but **lazy-loaded**.

**Behavior**:
- If module A imports B, and B imports A:
  - A is loaded first
  - B is loaded (sees A as partially initialized)
  - A completes loading

**Example**:
```izi
// a.iz
import { b } from "./b.iz";
export var a = 1;

// b.iz
import { a } from "./a.iz";  // May be undefined initially
export var b = 2;
```

**Rationale**:
- Real-world code has circular dependencies
- Lazy loading resolves most cases
- Errors only if actually used before initialized

**Warning**: Avoid circular dependencies when possible (code smell).

---

### 7.4 Default Exports
**Status**: 🔓 **OPEN**

**Question**: Should we support default exports?

**Proposal**:
```izi
// module.iz
export default fn myFunction() { ... }

// main.iz
import myFunction from "./module.iz";
```

**Pros**:
- Common in JavaScript
- Convenient for single-export modules

**Cons**:
- Less explicit
- Can lead to naming confusion

**Decision**: Defer to v0.2+. Not needed for v0.1.

---

## 8. Standard Library

### 8.1 Stdlib Philosophy
**Status**: 🔒 **FROZEN**

**Decision**: **Batteries-included** standard library.

**Rationale**:
- Productivity over minimalism
- Real-world programs need I/O, math, strings, collections
- Should not require external packages for basic tasks

**Included Modules** (v0.1):
- `std.math` - Mathematical functions (sqrt, sin, cos, etc.)
- `std.string` - String manipulation (split, trim, etc.)
- `std.array` - Array operations (map, filter, reduce)
- `std.io` - File I/O (readFile, writeFile)

**Future Modules** (v0.2+):
- `std.json` - JSON parsing
- `std.regex` - Regular expressions
- `std.time` - Date/time utilities
- `std.http` - HTTP client (v0.3+)
- `std.async` - Concurrency primitives (v0.3+)

**Implications**:
- Larger binary size
- More code to maintain
- Easier onboarding

---

### 8.2 Global Namespace
**Status**: 🔓 **OPEN** (needs improvement)

**Current Behavior**: All imported functions become global.

**Problem**:
```izi
import { sqrt } from "std.math";
sqrt(16);  // Works

// But also:
var sqrt = 42;  // Oops, shadowed the function!
```

**Proposed Fix** (v0.2):
- Namespaced imports: `std.math.sqrt(16)`
- Explicit global: `import { sqrt as globalSqrt } from "std.math"`

**Decision**: Fix in v0.2 (breaking change).

---

### 8.3 Native Functions
**Status**: 🔒 **FROZEN** (interface)

**Decision**: Native functions written in C++ and registered at startup.

**Interface**:
```cpp
Value nativeSqrt(const std::vector<Value>& args) {
    if (args.size() != 1) throw RuntimeError("sqrt expects 1 argument");
    double x = asNumber(args[0]);
    return std::sqrt(x);
}

// Register
registerNativeFunction("sqrt", nativeSqrt);
```

**Rationale**:
- Performance (C++ is fast)
- Access to system APIs
- Extensible (users can add native modules)

**Implications**:
- FFI (foreign function interface) needed for user extensions
- Requires C++ compiler for extensions

---

## 9. Syntax Choices

### 9.1 Statement Terminators
**Status**: 🔒 **FROZEN**

**Decision**: Semicolons (`;`) are **optional** but **recommended**.

**Rules**:
- Semicolon can be omitted at end of line
- Required to separate multiple statements on one line

**Examples**:
```izi
// ✅ Valid (no semicolons)
var x = 42
var y = 3.14

// ✅ Valid (with semicolons)
var x = 42;
var y = 3.14;

// ✅ Valid (multiple statements on one line)
var x = 42; var y = 3.14;

// ❌ Invalid (ambiguous)
var x = 42 var y = 3.14
```

**Rationale**:
- Flexibility (Python-style or C-style)
- Avoids JavaScript ASI (automatic semicolon insertion) confusion

---

### 9.2 Comments
**Status**: 🔒 **FROZEN** (syntax)

**Decision**: C-style comments: `//` for single-line, `/* */` for multi-line.

**Syntax**:
```izi
// Single-line comment
var x = 42; // Inline comment

/*
  Multi-line comment
  Spans multiple lines
*/
```

**Current Issue**: ⚠️ Lexer bug with `//` comments (crashes).

**Fix Required**: High priority for v0.1.

---

### 9.3 Variable Declaration
**Status**: 🔒 **FROZEN**

**Decision**: Use `var` keyword (not `let`/`const`).

**Syntax**:
```izi
var x = 42;          // Mutable
var y = "hello";     // Also mutable
```

**Rationale**:
- Simple (one keyword, not three)
- All variables are mutable (no const)
- Can add immutability later (v1.0+)

**Alternatives**:
1. **JavaScript** (`let`, `const`, `var`) - Too complex for beginners
2. **Rust** (`let` mutable, `let mut` immutable) - Backwards from convention

**Future**: May add `const` in v1.0+ for immutable bindings.

---

### 9.4 Function Declaration
**Status**: 🔒 **FROZEN**

**Decision**: Use `fn` keyword (not `function` or `def`).

**Syntax**:
```izi
fn greet(name) {
    return "Hello, " + name;
}
```

**Rationale**:
- Short and clear (`fn` < `function`)
- Distinct from `fun` (Kotlin) and `func` (Swift)
- Matches Rust syntax

---

### 9.5 String Interpolation
**Status**: 🔒 **FROZEN**

**Decision**: F-strings with `f"..."` syntax (Python-style).

**Syntax**:
```izi
var name = "Alice";
var age = 30;
print(f"Hello, {name}! You are {age} years old.");
```

**Rationale**:
- More readable than concatenation
- Familiar to Python developers
- Supports expressions inside `{}`

**Alternatives**:
1. **JavaScript** (backticks) - Conflicts with Markdown
2. **Ruby** (`#{}`) - Less intuitive

---

## 10. Performance vs. Simplicity

### 10.1 Optimization Philosophy
**Status**: 🔒 **FROZEN**

**Decision**: **Simplicity first, optimize later** (v0.1-v0.2).

**Rationale**:
- Premature optimization is the root of all evil (Knuth)
- Correctness > Performance initially
- Profile before optimizing

**Optimization Schedule**:
- **v0.1**: No optimizations (interpreter only)
- **v0.2**: Fix VM bugs, basic optimizations
- **v0.3**: Constant folding, dead code elimination, inline caching
- **v1.0+**: JIT compilation (if needed)

---

### 10.2 Performance Goals
**Status**: 🔓 **OPEN**

**Target**: Match **Python 3.x** performance by v0.3.

**Benchmarks** (planned):
- Fibonacci (recursion)
- Prime sieve (loops)
- JSON parsing (I/O)
- Sorting (algorithms)

**Comparison**:
```
Language       | Speed    | Startup Time
---------------|----------|-------------
C              | 1x       | 0ms
Python 3.11    | 80x      | 20ms
IziLang v0.1   | ???x     | ???ms (TBD)
IziLang v0.3   | 80x      | 30ms (goal)
```

---

## 11. Backward Compatibility

### 11.1 Pre-v1.0 Policy
**Status**: 🔒 **FROZEN**

**Decision**: Breaking changes are **allowed** before v1.0.

**Rules**:
- **v0.x**: Breaking changes permitted in minor versions
- Document all breaking changes in changelog
- Provide migration guide for major changes

**Rationale**:
- Allows rapid iteration
- Avoid locking in bad decisions early

---

### 11.2 Post-v1.0 Policy
**Status**: 🔒 **FROZEN**

**Decision**: Adopt **Semantic Versioning (SemVer)** after v1.0.

**Rules**:
```
MAJOR.MINOR.PATCH

MAJOR: Breaking changes (increment for any incompatibility)
MINOR: New features (backward compatible)
PATCH: Bug fixes (backward compatible)
```

**Deprecation Policy**:
1. Feature marked deprecated in MAJOR.x
2. Warning printed at runtime
3. Removed in MAJOR+1.0

**Example**:
- v1.0: Feature X works
- v2.0: Feature X deprecated (warning)
- v3.0: Feature X removed

---

### 11.3 Syntax Stability
**Status**: 🔓 **OPEN** (freeze at v0.2)

**Plan**: Freeze syntax in v0.2, only add (not change).

**Frozen After v0.2**:
- Keywords (fn, var, if, while, etc.)
- Operators (+, -, *, /, ==, etc.)
- Statement syntax
- Expression syntax

**Can Still Add**:
- New keywords (if not conflicting)
- New operators (if not ambiguous)
- New statement types

---

## Summary of Critical Decisions

| Decision | Status | Target |
|----------|--------|--------|
| **Execution Model** | 🔓 Open | v0.1 (review) |
| **Type System** | 🔒 Frozen | Dynamic (v0.1), Gradual (v0.2+) |
| **Memory Management** | 🔓 Open | v0.3 (GC decision needed) |
| **Error Handling** | 🔒 Frozen | Exceptions with try/catch |
| **Concurrency** | 🔓 Open | v0.3 (async/await recommended) |
| **Module System** | 🔒 Frozen | ES6-style imports |
| **Stdlib Philosophy** | 🔒 Frozen | Batteries-included |
| **Syntax** | 🔒 Frozen | C-like with modern features |
| **Backward Compat** | 🔒 Frozen | Breaking until v1.0, SemVer after |

---

## Decision Review Process

1. **Proposal**: Open GitHub issue with `design-decision` label
2. **Discussion**: Community feedback (2 weeks minimum)
3. **Decision**: Core team votes
4. **Documentation**: Add to this file
5. **Implementation**: Code changes
6. **Freeze**: Mark as 🔒 when stable

---

## Revision History

| Date | Section | Change | Reason |
|------|---------|--------|--------|
| 2026-02-11 | All | Initial version | Project checkpoint |

---

**Document maintained by**: IziLang Development Team  
**Questions**: Open a GitHub discussion with tag `design`

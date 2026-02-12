# IziLang Feature Inventory

**Version**: 1.0  
**Date**: February 2026  
**Purpose**: Comprehensive catalog of all language features (implemented, in-progress, and planned)

---

## Table of Contents

1. [Implemented Features](#implemented-features)
2. [In Progress Features](#in-progress-features)
3. [Planned Features](#planned-features)
4. [Under Evaluation](#under-evaluation)
5. [Explicitly Excluded](#explicitly-excluded)
6. [Feature Gaps & Risks](#feature-gaps--risks)

---

## Feature Status Legend

- ✅ **Complete** — Fully implemented, tested, documented
- 🚧 **In Progress** — Partially implemented, being worked on
- 📋 **Planned** — Approved for implementation, not started
- 🤔 **Evaluating** — Under discussion, not yet approved
- ❌ **Excluded** — Explicitly decided against
- ⏸️ **Deferred** — Approved but postponed to later version

---

## Implemented Features

### Core Language (v0.2)

#### 1. Data Types ✅
**Status**: Complete since v0.1

**Primitive Types**:
- `nil` — Represents absence of value
- `bool` — `true` or `false`
- `number` — 64-bit IEEE 754 floating point
- `string` — UTF-8 encoded text

**Collection Types**:
- `array` — Ordered, mutable, 0-indexed lists
- `map` — Hash table with string keys

**Examples**:
```izi
var nothing = nil;
var flag = true;
var count = 42;
var pi = 3.14159;
var name = "Alice";
var numbers = [1, 2, 3, 4, 5];
var person = { name: "Bob", age: 30 };
```

#### 2. Operators ✅
**Status**: Complete since v0.1

**Arithmetic**: `+`, `-`, `*`, `/`  
**Comparison**: `<`, `>`, `<=`, `>=`, `==`, `!=`  
**Logical**: `and`, `or`, `!` (not)  
**Unary**: `-` (negation), `!` (boolean NOT)  
**Assignment**: `=`  
**Indexing**: `[...]` (arrays and maps)

**Examples**:
```izi
var x = 10 + 5;        // 15
var isAdult = age >= 18;
var result = x > 0 and y < 100;
var negated = -x;
var notFlag = !true;   // false
array[0] = 42;
map["key"] = "value";
```

#### 3. Control Flow ✅
**Status**: Complete since v0.2

**If/Else Statements**:
```izi
if (condition) {
    doSomething();
} else {
    doSomethingElse();
}
```

**While Loops**:
```izi
while (condition) {
    doWork();
}
```

**For Loops**:
```izi
for (var i = 0; i < 10; i = i + 1) {
    print(i);
}
```

**Break and Continue**:
```izi
while (true) {
    if (shouldStop) break;
    if (shouldSkip) continue;
}
```

#### 4. Functions ✅
**Status**: Complete since v0.1

**Function Declaration**:
```izi
fn add(a, b) {
    return a + b;
}
```

**Anonymous Functions**:
```izi
var multiply = fn(x, y) {
    return x * y;
};
```

**Closures**:
```izi
fn makeCounter() {
    var count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
}
```

**Higher-Order Functions**:
```izi
fn applyTwice(f, x) {
    return f(f(x));
}
```

**Features**:
- First-class functions (can be passed as values)
- Lexical scoping (closures capture environment)
- Recursion support
- Multiple return values via arrays/maps

#### 5. Pattern Matching ✅
**Status**: Complete since v0.2

**Match Expressions**:
```izi
var result = match value {
    case 1 => "one",
    case 2 => "two",
    case 3 => "three",
    case _ => "other"
};
```

**Guards**:
```izi
match (x, y) {
    case (a, b) if a > b => "first is larger",
    case (a, b) if a < b => "second is larger",
    case _ => "equal"
}
```

**Tuple Matching**:
```izi
match (status, code) {
    case ("ok", 200) => handleSuccess(),
    case ("error", code) => handleError(code),
    case _ => handleUnknown()
}
```

#### 6. Exception Handling ✅
**Status**: Complete since v0.2

**Try/Catch/Finally**:
```izi
try {
    var data = readFile("config.json");
    processData(data);
} catch (err) {
    print("Error: " + err.message);
    logError(err);
} finally {
    cleanup();
}
```

**Throw Exceptions**:
```izi
fn divide(a, b) {
    if (b == 0) {
        throw "Division by zero";
    }
    return a / b;
}
```

**Features**:
- Exception propagation
- Stack traces with function names
- Custom error messages
- Finally block always executes

#### 7. String Interpolation ✅
**Status**: Complete since v0.2

**F-Strings**:
```izi
var name = "Alice";
var age = 30;
print(f"Hello, {name}! You are {age} years old.");
// Output: Hello, Alice! You are 30 years old.
```

**Expression Evaluation**:
```izi
print(f"2 + 2 = {2 + 2}");
// Output: 2 + 2 = 4
```

#### 8. Comments ✅
**Status**: Complete since v0.1

**Single-Line Comments**:
```izi
// This is a comment
var x = 42; // Inline comment
```

**Multi-Line Comments**:
```izi
/*
  This is a multi-line comment
  Spans multiple lines
*/
```

#### 9. Module System ✅
**Status**: Complete since v0.2

**Named Imports**:
```izi
import { sqrt, pow } from "math";
var result = sqrt(16);
```

**Wildcard Imports**:
```izi
import * as math from "math";
var result = math["sqrt"](16);
```

**Exports**:
```izi
export fn myFunction() {
    return 42;
}

export var myVariable = "hello";
```

**Features**:
- ES6-style import/export syntax
- Namespace objects for wildcard imports
- Circular dependency handling (lazy loading)
- Built-in module resolution (std.* modules)
- Relative path imports (./module, ../module)

#### 10. Standard Library ✅
**Status**: 46 functions across 4 modules

**std.math** (13 items):
```izi
import { sqrt, pow, sin, cos, tan, floor, ceil, round, abs, min, max, PI, E } from "math";
```

**std.string** (10 functions):
```izi
import { substring, split, toUpper, toLower, trim, replace, indexOf, startsWith, endsWith, join } from "string";
```

**std.array** (7 functions):
```izi
import { map, filter, reduce, sort, reverse, concat, slice } from "array";
```

**std.io** (4 functions):
```izi
import { readFile, writeFile, appendFile, fileExists } from "io";
```

**Global Functions** (2):
```izi
print(value);      // Output to stdout
len(collection);   // Get length of array/map/string
```

---

## In Progress Features

### v0.3 Features (35% Complete)

#### 1. Gradual Typing System 🚧
**Status**: Type system designed, parser support needed

**Type Annotations**:
```izi
var x: number = 42;
var name: string = "Alice";
var items: array<number> = [1, 2, 3];
```

**Function Signatures**:
```izi
fn add(a: number, b: number): number {
    return a + b;
}
```

**Progress**:
- ✅ TypeAnnotation struct designed
- ✅ AST extensions added
- ⏸️ Parser support (not started)
- ⏸️ Type checking implementation
- ⏸️ Type inference (deferred to v0.4)

#### 2. Classes and OOP 🚧
**Status**: AST ready, implementation needed

**Class Declaration**:
```izi
class Person {
    var name: string;
    var age: number;
    
    fn constructor(name, age) {
        this.name = name;
        this.age = age;
    }
    
    fn greet() {
        print(f"Hello, I'm {this.name}");
    }
}
```

**Instantiation**:
```izi
var alice = Person("Alice", 30);
alice.greet();
```

**Progress**:
- ✅ ClassStmt AST node created
- ✅ PropertyExpr, SetPropertyExpr created
- ✅ ThisExpr created
- ⏸️ Parser support
- ⏸️ Interpreter implementation
- ⏸️ VM bytecode support
- ⏸️ Inheritance (deferred to v0.4)

#### 3. Semantic Analysis 🚧
**Status**: Core implemented, advanced features needed

**Implemented**:
- ✅ Unused variable detection
- ✅ Unreachable code detection (basic)
- ✅ Break/continue outside loop errors
- ✅ Return outside function errors
- ✅ Symbol table and scoping

**Not Yet Implemented**:
- ⏸️ Constant folding
- ⏸️ More precise dead code analysis
- ⏸️ Type inference hooks
- ⏸️ Flow-sensitive analysis

**Command**:
```bash
izi check src/    # Static analysis without execution
```

#### 4. Garbage Collection 🚧
**Status**: Design complete, implementation not started

**Current**: Reference counting via `std::shared_ptr`  
**Planned**: Mark-and-Sweep GC with generational extension

**Progress**:
- ✅ Design decision made
- ✅ Documentation written
- ⏸️ GC allocator infrastructure
- ⏸️ Root set tracking
- ⏸️ Mark phase
- ⏸️ Sweep phase
- ⏸️ Generational optimization

**Rationale**: Handle circular references, industry-standard approach

---

## Planned Features

### v1.0 — Foundation

#### 1. Stable Syntax Freeze 📋
**Target**: v0.2 (then frozen for v1.0)

**Goal**: No breaking syntax changes after v0.2

**Includes**:
- All keywords finalized
- All operators finalized
- Statement and expression syntax locked
- Can only add (not change) syntax post-v0.2

#### 2. Deterministic Compilation 📋
**Target**: v0.3

**Goal**: Same input always produces same output

**Features**:
- Reproducible builds
- No timestamp in output
- Deterministic module loading order
- Consistent error messages

#### 3. Complete Standard Library 📋
**Target**: v0.3

**Modules to Add**:
- `std.json` — JSON parsing and serialization
- `std.regex` — Regular expressions
- `std.time` — Date/time utilities
- `std.env` — Environment variables
- `std.process` — Process execution
- `std.net` — Basic networking (v0.4)

**Total Target**: 100+ functions by v1.0

#### 4. Enhanced Error Diagnostics 📋
**Target**: v0.3

**Improvements**:
- Source file names in stack traces
- Line numbers in all traces
- Color-coded output (respects NO_COLOR)
- Suggestion system (common fixes)
- Error codes for documentation lookup

#### 5. Linux-First Support 📋
**Target**: v1.0 (already achieved)

**Status**: ✅ Already works on Linux, macOS, Windows

**Focus**: Ensure excellent Linux experience
- Package managers (apt, yum)
- Docker images
- System integration
- Man pages

### v1.1 — Productivity

#### 6. REPL (Interactive Shell) 📋
**Target**: v0.2

**Features**:
- Multi-line input support
- History navigation (up/down arrows)
- Tab completion
- Syntax highlighting
- Special commands (`:help`, `:quit`, `:clear`)

**Example**:
```bash
$ izi repl
IziLang 1.0.0 REPL
> var x = 42;
> print(x + 8);
50
> :quit
```

#### 7. Code Formatter 📋
**Target**: v0.2

**Features**:
```bash
izi fmt file.iz          # Format file in-place
izi fmt --check file.iz  # Check without modifying
izi fmt src/             # Format directory
```

**Style**:
- 4-space indentation (configurable)
- Consistent spacing around operators
- Configurable via `.izifmt.toml`
- Opinionated defaults (minimal configuration)

#### 8. Better Error Messages 📋
**Target**: Ongoing

**Improvements**:
- More specific error types
- Better suggestions
- Related information (similar names)
- Documentation links

**Example**:
```
Error E0308: Type mismatch at line 5, column 12:
  5 |     var result = name + age;
    |                  ^^^^ ^^^ expected string, found number

help: Convert `age` to string: name + string(age)
docs: https://izilang.org/errors/E0308
```

#### 9. Testing Framework 📋
**Target**: v0.2

**Built-in Testing**:
```izi
// test_math.iz
import { assert, assertEqual } from "test";

test("addition works", fn() {
    assertEqual(2 + 2, 4);
});

test("division by zero", fn() {
    assertThrows(fn() { var x = 10 / 0; });
});
```

**Runner**:
```bash
izi test                 # Run all tests
izi test test_math.iz    # Run specific file
izi test --watch         # Re-run on file changes
```

#### 10. Debug Symbols & Tooling 📋
**Target**: v0.3

**Features**:
- Line number tracking in bytecode
- Variable name preservation
- Stack frame inspection
- Breakpoint support (via debugger)

### v1.2 — Ecosystem

#### 11. Package Manager 📋
**Target**: v0.2

**Commands**:
```bash
izi-pkg init             # Create izi.json
izi-pkg install express  # Install package
izi-pkg publish          # Publish to registry
izi-pkg search http      # Search packages
```

**Manifest** (`izi.json`):
```json
{
  "name": "myapp",
  "version": "1.0.0",
  "dependencies": {
    "express": "^1.0.0",
    "database": "^2.3.0"
  }
}
```

**Registry**: Hosted package repository (like npm, crates.io)

#### 12. FFI (Foreign Function Interface) 📋
**Target**: v0.4

**Goal**: Call C libraries from IziLang

**Example**:
```izi
import { ffi } from "ffi";

var libc = ffi.load("libc.so.6");
var strlen = libc.function("strlen", [ffi.string], ffi.int);

print(strlen("hello"));  // 5
```

**Use Cases**:
- System APIs
- Existing C libraries
- Performance-critical code
- Hardware access

#### 13. LSP (Language Server Protocol) 📋
**Target**: v0.2 (MVP), v0.3 (complete)

**Features**:
- ✅ Framework exists (skeleton)
- ⏸️ Diagnostics (syntax errors, warnings)
- ⏸️ Auto-completion (variables, functions, keywords)
- ⏸️ Hover information (type info, docs)
- ⏸️ Go-to-definition
- ⏸️ Find references
- ⏸️ Rename symbol
- ⏸️ Code actions (quick fixes)

**Integration**: VS Code, Vim, Emacs, Sublime, etc.

#### 14. Formatter & Linter 📋
**Target**: v0.2

**Linter Features**:
```bash
izi lint src/            # Check for issues
izi lint --fix src/      # Auto-fix where possible
```

**Checks**:
- Unused variables
- Unreachable code
- Naming conventions
- Complexity metrics
- Style violations

**Configurable via** `.izilint.toml`

#### 15. Cross-Platform Support 📋
**Target**: v1.0 (already achieved for Linux/macOS/Windows)

**Extended Platforms**:
- FreeBSD
- NetBSD
- OpenBSD
- ARM64 Linux
- WebAssembly (future)

### v2.0 — Advanced

#### 16. Async/Await Concurrency 📋
**Target**: v0.3

**Syntax**:
```izi
async fn fetchData(url) {
    var response = await http.get(url);
    var data = await response.json();
    return data;
}

var data = await fetchData("https://api.example.com/users");
```

**Model**: Event loop with non-blocking I/O

**Standard Library**:
- `std.async` module
- Async file I/O
- Async HTTP client
- Promise/Future types
- Task scheduler

#### 17. Optimization Passes 📋
**Target**: v0.3

**Optimizations**:
- Constant folding (`2 + 3` → `5`)
- Dead code elimination
- Inline caching (property lookups)
- Function inlining (small functions)
- Loop unrolling (small loops)

**Goal**: 2-3x performance improvement

#### 18. JIT Compilation 📋
**Target**: v1.0+

**Goal**: Compile hot code paths to native

**Approach**:
- Profile-guided (identify hot code)
- Incremental (compile gradually)
- Fallback to interpreter (always safe)

**Performance Target**: 10x improvement for hot loops

---

## Under Evaluation

### Features Being Considered

#### 1. Static Type Inference 🤔
**Status**: Deferred to v0.4+

**Proposal**: Infer types without annotations

**Example**:
```izi
var x = 42;           // Inferred: number
var name = "Alice";   // Inferred: string

fn add(a, b) {        // Inferred: (number, number) -> number
    return a + b;
}
```

**Pros**:
- Less boilerplate than full annotations
- Still get type safety
- Similar to TypeScript, Kotlin

**Cons**:
- Complex implementation (Hindley-Milner)
- May infer wrong types (surprising)
- Errors far from source

**Decision**: Evaluate based on gradual typing adoption

#### 2. Generics / Templates 🤔
**Status**: Under discussion

**Proposal**: Parameterized types

**Example**:
```izi
fn identity<T>(x: T): T {
    return x;
}

class Box<T> {
    var value: T;
    
    fn get(): T {
        return this.value;
    }
}
```

**Pros**:
- Type-safe collections
- Reusable algorithms
- Better standard library

**Cons**:
- Complex type system
- Harder to learn
- Compilation complexity

**Decision**: Wait for user feedback on gradual typing first

#### 3. Traits / Interfaces 🤔
**Status**: Under discussion

**Proposal**: Define behavior contracts

**Example**:
```izi
trait Drawable {
    fn draw();
}

class Circle implements Drawable {
    fn draw() {
        print("Drawing circle");
    }
}
```

**Pros**:
- Polymorphism without inheritance
- Clear contracts
- Testable (mocking)

**Cons**:
- More concepts to learn
- Complexity in type system

**Decision**: Consider for v0.4 if OOP is successful

#### 4. Macros / Metaprogramming 🤔
**Status**: Low priority

**Proposal**: Code generation at compile time

**Example**:
```izi
macro unless(condition, body) {
    if (!condition) {
        body
    }
}

unless (isValid) {
    throw "Invalid data";
}
```

**Pros**:
- Domain-specific languages
- Reduce boilerplate
- Code generation

**Cons**:
- Complex to implement
- Hard to debug
- Can make code unreadable

**Decision**: Not planned for v1.x (maybe v2.0+)

#### 5. Operator Overloading 🤔
**Status**: Under consideration

**Proposal**: Define operators for custom types

**Example**:
```izi
class Vector {
    var x, y;
    
    fn __add__(other) {
        return Vector(this.x + other.x, this.y + other.y);
    }
}

var v1 = Vector(1, 2);
var v2 = Vector(3, 4);
var v3 = v1 + v2;  // Uses __add__
```

**Pros**:
- Natural syntax for math
- Used in Python, C++, Rust
- Convenient for DSLs

**Cons**:
- Can be abused (unclear semantics)
- Violates "explicit over implicit"
- Harder to debug

**Decision**: Probably not for v1.0 (conflicts with "easy" principle)

#### 6. Reflection API 🤔
**Status**: Low priority

**Proposal**: Inspect types and structure at runtime

**Example**:
```izi
var obj = Person("Alice", 30);
var fields = reflect.fields(obj);  // ["name", "age"]
var type = reflect.type(obj);       // "Person"
```

**Pros**:
- Serialization
- ORMs and frameworks
- Testing utilities

**Cons**:
- Performance overhead
- Security concerns
- Complexity

**Decision**: Not for v1.0, maybe v1.x if needed

#### 7. Pattern Guards (Advanced) 🤔
**Status**: Under consideration

**Proposal**: More expressive pattern matching

**Example**:
```izi
match user {
    case { role: "admin", active: true } => grantAccess(),
    case { role: "user", credits: c } if c > 0 => limitedAccess(),
    case _ => denyAccess()
}
```

**Status**: Already supported! (guards with `if`)

**Decision**: Current implementation sufficient

---

## Explicitly Excluded

### Features We Will NOT Add

#### 1. Manual Memory Management ❌
**Reason**: Safety over control

- No `malloc`/`free`
- No `new`/`delete`
- No pointer arithmetic
- No `unsafe` blocks

**Alternative**: Use FFI for performance-critical code

#### 2. Multiple Inheritance ❌
**Reason**: Complexity and diamond problem

**Alternative**: Use composition or traits (future)

#### 3. Goto Statements ❌
**Reason**: Spaghetti code, hard to reason about

**Alternative**: Use proper control flow

#### 4. Preprocessor / Macros (v1.0) ❌
**Reason**: Makes code hard to understand

**Alternative**: Functions and metaprogramming (v2.0+)

#### 5. Implicit Type Coercion ❌
**Reason**: Leads to bugs (JavaScript's `"5" + 3 === "53"`)

**Decision**: Frozen, will never be added

#### 6. Global Variables (Uncontrolled) ❌
**Reason**: Hidden dependencies, hard to test

**Alternative**: Pass parameters explicitly, use modules

#### 7. Eval (Arbitrary Code Execution) ❌
**Reason**: Security risk

**Alternative**: Use proper APIs

#### 8. Automatic Semicolon Insertion ❌
**Reason**: Leads to surprising bugs (JavaScript)

**Decision**: Semicolons are optional but explicit

---

## Feature Gaps & Risks

### Critical Gaps (Must Address for v1.0)

#### 1. ⚠️ VM Stability
**Issue**: Bytecode VM has bugs, not production-ready

**Impact**: Performance mode unavailable

**Mitigation**:
- Use interpreter as default (stable)
- Fix VM incrementally
- Mark VM as experimental

**Timeline**: Fix by v0.3

#### 2. ⚠️ Concurrency Model
**Issue**: No threading or async support

**Impact**: Cannot handle concurrent I/O efficiently

**Mitigation**:
- Single-threaded is sufficient for v1.0
- Add async/await in v0.3
- Document limitations

**Timeline**: Decide by Q2 2026, implement Q3 2026

#### 3. ⚠️ Package Ecosystem
**Issue**: No package registry or manager

**Impact**: Hard to share and reuse code

**Mitigation**:
- Manual dependencies for v1.0
- Build package manager for v0.2
- Create registry for v0.3

**Timeline**: MVP by v0.2

### Medium-Priority Gaps

#### 4. ⏸️ Static Analysis
**Issue**: Limited compile-time checking

**Mitigation**:
- Add `izi check` command
- Semantic analyzer (v0.3)
- Gradual typing (v0.3+)

#### 5. ⏸️ Debugger
**Issue**: No breakpoint debugging

**Mitigation**:
- Print debugging works
- Stack traces available
- Full debugger in v0.3

#### 6. ⏸️ Cross-Compilation
**Issue**: Must build on target platform

**Mitigation**:
- Not critical for v1.0
- Add in v1.2

### Low-Priority Gaps

#### 7. Web/Browser Support
**Issue**: No WebAssembly target

**Status**: Not needed for v1.0, consider for v2.0

#### 8. Mobile Platforms
**Issue**: No iOS/Android support

**Status**: Out of scope (desktop/server focus)

---

## Prioritization Framework

### Must-Have for v1.0
1. Stable syntax
2. Working interpreter
3. Core language features (functions, closures, control flow)
4. Module system
5. Standard library (minimal but useful)
6. Good error messages
7. Multi-platform builds
8. Documentation

### Should-Have for v1.0
1. Pattern matching
2. Exception handling
3. REPL
4. Code formatter
5. LSP server (basic)
6. Test framework

### Nice-to-Have for v1.0
1. Gradual typing
2. Classes/OOP
3. Package manager
4. Performance optimizations
5. Debugger

### Post-v1.0
1. Async/await
2. Advanced type features
3. JIT compilation
4. Reflection
5. Macros

---

## Version Mapping

| Feature | v0.1 | v0.2 | v0.3 | v1.0 | v1.x | v2.0 |
|---------|------|------|------|------|------|------|
| Core language | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Functions/closures | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Pattern matching | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| Exceptions | - | ✅ | ✅ | ✅ | ✅ | ✅ |
| Module system | - | ✅ | ✅ | ✅ | ✅ | ✅ |
| Standard library | Basic | ✅ | ✅ | ✅ | ✅ | ✅ |
| Classes/OOP | - | - | 🚧 | ✅ | ✅ | ✅ |
| Gradual typing | - | - | 🚧 | ✅ | ✅ | ✅ |
| REPL | - | 📋 | ✅ | ✅ | ✅ | ✅ |
| Formatter | - | 📋 | ✅ | ✅ | ✅ | ✅ |
| LSP | - | 📋 | ✅ | ✅ | ✅ | ✅ |
| Package manager | - | 📋 | 📋 | ✅ | ✅ | ✅ |
| Async/await | - | - | 📋 | - | 📋 | ✅ |
| JIT compilation | - | - | - | - | 🤔 | 📋 |
| Reflection | - | - | - | - | 🤔 | 📋 |

---

## Conclusion

IziLang has a **solid foundation** of implemented features (v0.1-v0.2), with clear plans for gradual evolution (v0.3-v1.0). The feature set balances **pragmatism** (useful features) with **simplicity** (avoid bloat).

**Key Strengths**:
- Core language complete and stable
- Rich standard library (46 functions)
- Modern features (pattern matching, closures, modules)
- Clear roadmap for missing pieces

**Key Focus Areas**:
- Stabilize VM for performance
- Add OOP and gradual typing (v0.3)
- Build tooling ecosystem (LSP, formatter, package manager)
- Grow standard library

**Guiding Principle**: Add features that make IziLang **easier and more productive**, not just more powerful.

---

**Document Status**: Living Document  
**Last Updated**: February 2026  
**Maintained By**: IziLang Core Team  
**Feedback**: GitHub Issues with `feature-inventory` label

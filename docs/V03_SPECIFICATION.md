# IziLang v0.3 Language Specification

## Overview

IziLang v0.3 introduces **gradual typing**, **classes**, **semantic analysis**, and **mark-and-sweep garbage collection** to provide a more robust, performant, and scalable language.

---

## 1. Gradual Typing System

### 1.1 Philosophy

IziLang v0.3 implements **optional type annotations** without breaking backward compatibility:

- Unannotated code runs exactly like v0.2 (fully dynamic)
- Type annotations enable **compile-time checking** via `izi check`
- Runtime remains dynamic regardless of annotations
- Type errors are reported before execution

### 1.2 Type Annotation Syntax

#### Variables

```izi
// Without type annotation (dynamic)
var x = 10;

// With type annotation
var x: Number = 10;
var name: String = "Alice";
var flag: Bool = true;
var empty: Nil = nil;
```

#### Functions

```izi
// Without type annotations (v0.2 style)
fn add(a, b) {
    return a + b;
}

// With type annotations
fn add(a: Number, b: Number): Number {
    return a + b;
}

// Void return type
fn greet(name: String): Void {
    print("Hello, " + name);
}

// Mixed (some parameters typed, some not)
fn process(data, verbose: Bool): String {
    // ...
}
```

### 1.3 Built-in Types

| Type | Description | Example |
|------|-------------|---------|
| `Number` | 64-bit floating-point | `42`, `3.14` |
| `String` | UTF-8 string | `"hello"` |
| `Bool` | Boolean | `true`, `false` |
| `Nil` | Null value | `nil` |
| `Array` | Dynamic array | `[1, 2, 3]` |
| `Map` | Hash map | `{a: 1, b: 2}` |
| `Any` | Any type (default for unannotated) | - |
| `Void` | No return value | - |

### 1.4 Complex Types

#### Array Types

```izi
var numbers: Array<Number> = [1, 2, 3];
var names: Array<String> = ["Alice", "Bob"];
```

#### Map Types

```izi
var ages: Map<String, Number> = {
    "Alice": 30,
    "Bob": 25
};
```

#### Function Types

```izi
// Function as parameter
fn apply(f: Function(Number) -> Number, x: Number): Number {
    return f(x);
}

// Function as variable
var transformer: Function(String) -> String = fn(s) {
    return toUpper(s);
};
```

### 1.5 Type Checking Rules

1. **Exact Match**: Assignment must match annotated type exactly
   ```izi
   var x: Number = 10;      // OK
   var y: Number = "hello"; // ERROR: Type mismatch
   ```

2. **Any Compatibility**: `Any` is compatible with all types
   ```izi
   var data: Any = 42;      // OK
   data = "string";         // OK
   ```

3. **Function Signature**: Parameters and return type must match
   ```izi
   fn square(x: Number): Number {
       return x * x;
   }
   
   var result: Number = square(5);      // OK
   var bad: String = square(5);         // ERROR
   var worse: Number = square("text");  // ERROR
   ```

4. **Array Element Types**: All elements must match declared type
   ```izi
   var nums: Array<Number> = [1, 2, 3];  // OK
   var mixed: Array<Number> = [1, "2"];  // ERROR
   ```

### 1.6 Type Inference (Future)

Currently, IziLang only checks **explicitly annotated** types. Full type inference is planned for v0.4+.

---

## 2. Classes and Object-Oriented Programming

### 2.1 Class Declaration

```izi
class Point {
    var x: Number;
    var y: Number;
    
    fn constructor(x: Number, y: Number) {
        this.x = x;
        this.y = y;
    }
    
    fn distance(): Number {
        return sqrt(this.x * this.x + this.y * this.y);
    }
    
    fn translate(dx: Number, dy: Number): Void {
        this.x = this.x + dx;
        this.y = this.y + dy;
    }
}
```

### 2.2 Creating Instances

```izi
var p1 = Point(3, 4);
var p2 = Point(0, 0);

print(p1.distance());  // 5.0
p1.translate(1, 1);
print(p1.x);  // 4
```

### 2.3 The `this` Keyword

- `this` refers to the current instance inside methods
- Cannot be used outside of class methods
- Automatically bound when methods are called

```izi
class Counter {
    var count: Number;
    
    fn constructor() {
        this.count = 0;
    }
    
    fn increment(): Void {
        this.count = this.count + 1;
    }
    
    fn getValue(): Number {
        return this.count;
    }
}

var c = Counter();
c.increment();
print(c.getValue());  // 1
```

### 2.4 Explicitly Excluded (v0.3)

To keep v0.3 scope reasonable, the following are **not included**:

- ❌ Inheritance
- ❌ Interfaces / traits
- ❌ Visibility modifiers (public/private)
- ❌ Static methods
- ❌ Operator overloading

These features are deferred to v0.4+.

---

## 3. Semantic Analysis

### 3.1 The `izi check` Command

Run semantic analysis without executing code:

```bash
izi check program.iz
```

### 3.2 Static Warnings

#### Unused Variables

```izi
fn example() {
    var x = 10;  // WARNING: Unused variable 'x'
    return 5;
}
```

#### Dead Code

```izi
fn process(): Number {
    return 42;
    print("Never executed");  // WARNING: Unreachable code
}
```

#### Break/Continue Outside Loop

```izi
fn bad() {
    break;  // ERROR: Break statement outside of loop
}
```

#### Return Outside Function

```izi
return 10;  // ERROR: Return statement outside of function
```

### 3.3 Type Errors (with annotations)

```izi
fn square(x: Number): Number {
    return x * x;
}

var result: String = square(5);  // ERROR: Type mismatch
```

---

## 4. Garbage Collection

### 4.1 Mark-and-Sweep GC

IziLang v0.3 replaces reference counting with a **mark-and-sweep garbage collector**:

- **Handles cyclic references** (v0.2 limitation removed)
- Automatic memory management
- Tunable collection frequency

### 4.2 GC Behavior

```izi
// Cyclic structures now work correctly
class Node {
    var value: Number;
    var next: Any;  // Can hold another Node
    
    fn constructor(v: Number) {
        this.value = v;
        this.next = nil;
    }
}

var a = Node(1);
var b = Node(2);
a.next = b;
b.next = a;  // Cycle! But GC handles it
```

When `a` and `b` go out of scope, the GC will:
1. Mark all reachable objects from roots
2. Sweep unreachable objects (including cycles)
3. Reclaim memory

### 4.3 GC Tuning (Advanced)

```bash
# Set GC trigger threshold (bytes)
export IZI_GC_THRESHOLD=1048576  # 1 MB

# Enable GC debug logging
export IZI_GC_DEBUG=1
```

---

## 5. VM Improvements

### 5.1 Import System

The bytecode VM now supports the full import system:

```izi
import { sqrt, PI } from "math";
import * as utils from "utils";

var result = sqrt(PI);
var data = utils.process();
```

### 5.2 Performance Baseline

Example benchmark results (v0.3):

| Operation | Interpreter | VM | Speedup |
|-----------|-------------|-----|---------|
| Fibonacci(30) | 2.3s | 0.8s | 2.9x |
| Loop (1M iterations) | 1.5s | 0.4s | 3.8x |
| Function calls (100K) | 0.9s | 0.3s | 3.0x |

Run your own benchmarks:

```bash
izi bench program.iz
```

---

## 6. Stack Traces

### 6.1 Complete Error Context

All runtime errors now include:
- File name
- Line number
- Column number
- Function call stack
- Code snippet with visual indicator

Example:

```
Runtime Error in 'example.iz':
  Line 15, Column 8:
    15 | var x = y + 5;
       |         ^
  Undefined variable 'y'
  
  Call stack:
    at processData() (example.iz:15:8)
    at main() (example.iz:22:5)
```

---

## 7. Migration from v0.2

### 7.1 Backward Compatibility

✅ **All v0.2 code runs unchanged in v0.3**

No breaking changes to:
- Syntax
- Semantics
- Standard library
- Runtime behavior

### 7.2 Opting Into New Features

Type annotations are **optional**:

```izi
// v0.2 style (still works)
fn add(a, b) {
    return a + b;
}

// v0.3 style (with types)
fn add(a: Number, b: Number): Number {
    return a + b;
}
```

Classes are **new syntax**:

```izi
// Before (v0.2): Objects via maps
var point = {
    x: 3,
    y: 4,
    distance: fn() { return sqrt(x*x + y*y); }
};

// After (v0.3): Proper classes
class Point {
    var x: Number;
    var y: Number;
    fn distance(): Number { return sqrt(this.x * this.x + this.y * this.y); }
}
```

---

## 8. Command-Line Interface

### 8.1 New Commands

```bash
# Run semantic analysis only (no execution)
izi check program.iz

# Run benchmarks
izi bench program.iz

# Existing commands (unchanged)
izi run program.iz
izi test
izi repl
```

### 8.2 New Flags

```bash
# Treat warnings as errors
izi check --strict program.iz

# Show GC statistics
izi run --gc-stats program.iz

# Force interpreter (bypass VM)
izi run --interp program.iz
```

---

## 9. Experimental Features

The following features are **experimental** in v0.3:

⚠️ **Bytecode VM**: Stable but still being optimized  
⚠️ **Type Inference**: Not yet implemented (use explicit annotations)  
⚠️ **Generics**: Design only, no implementation

---

## 10. Known Limitations

1. **Type Inference**: Must explicitly annotate; no automatic inference
2. **Class Features**: No inheritance, interfaces, or privacy modifiers
3. **GC Tuning**: Limited control over collection behavior
4. **Error Recovery**: Parser stops at first error (no multiple errors)

---

## 11. What's Next (v0.4)

Planned for next release:

- Full type inference
- Traits/interfaces for classes
- Async/await concurrency
- Package registry
- LSP server
- JIT compilation (experimental)

---

## References

- [v0.3 Milestone Issue](https://github.com/ismobaga/izilang/issues/XX)
- [CHANGELOG.md](../CHANGELOG.md)
- [Getting Started Guide](GETTING_STARTED.md)
- [API Reference](STANDARD_LIBRARY.md)

---

**Last Updated**: 2026-02-11  
**Version**: 0.3.0 (Draft)

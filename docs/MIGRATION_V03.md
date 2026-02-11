# Migration Guide: v0.2 → v0.3

## Introduction

IziLang v0.3 is **100% backward compatible** with v0.2. All existing code will run without modification. This guide shows you how to **opt into** new v0.3 features.

---

## Quick Summary

### What's New
- ✨ **Gradual Typing**: Optional type annotations
- ✨ **Classes**: Minimal OOP with fields and methods
- ✨ **Semantic Analysis**: `izi check` command
- ✨ **Mark-and-Sweep GC**: Handles cyclic references
- ✨ **Stack Traces**: Complete error context
- ✨ **VM Improvements**: Import system, performance baseline

### Breaking Changes
- **None!** All v0.2 code runs unchanged.

---

## Migration Steps

### Step 1: Test Your Code

Run your existing tests to ensure compatibility:

```bash
# Your v0.2 code should work as-is
izi test

# Or run specific files
izi run yourapp.iz
```

If any issues arise, please report them as bugs.

### Step 2: Run Semantic Analysis

Use the new `izi check` command to find potential issues:

```bash
izi check src/**/*.iz
```

This will warn you about:
- Unused variables
- Dead code after return
- Break/continue outside loops
- Return outside functions

**These are warnings only** — your code will still run.

### Step 3: Add Type Annotations (Optional)

Gradually add types to improve safety:

#### Before (v0.2)
```izi
fn calculateTotal(items) {
    var sum = 0;
    for item in items {
        sum = sum + item.price;
    }
    return sum;
}
```

#### After (v0.3)
```izi
fn calculateTotal(items: Array<Map<String, Number>>): Number {
    var sum: Number = 0;
    for item in items {
        sum = sum + item.price;
    }
    return sum;
}
```

**Benefits**:
- Catch type errors at check-time
- Self-documenting code
- Better IDE support (future)

### Step 4: Convert Objects to Classes (Optional)

If you use map-based objects, consider upgrading to classes:

#### Before (v0.2): Map-Based Objects
```izi
fn createPoint(x, y) {
    return {
        x: x,
        y: y,
        distance: fn() {
            return sqrt(x * x + y * y);
        }
    };
}

var p = createPoint(3, 4);
print(p.distance());
```

#### After (v0.3): Proper Classes
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
}

var p = Point(3, 4);
print(p.distance());
```

**Benefits**:
- Clearer syntax
- `this` binding
- Better performance (future optimizations)

### Step 5: Fix Cyclic References (if applicable)

If you avoided cyclic data structures in v0.2, you can now use them:

#### Before (v0.2): Manual Cycle Breaking
```izi
class Node {
    var value = 0;
    var next = nil;
}

var a = Node();
var b = Node();
a.next = b;
// b.next = a;  // AVOID: would leak memory in v0.2
```

#### After (v0.3): Cycles Work Fine
```izi
class Node {
    var value: Number;
    var next: Any;
    
    fn constructor(v: Number) {
        this.value = v;
        this.next = nil;
    }
}

var a = Node(1);
var b = Node(2);
a.next = b;
b.next = a;  // OK: GC handles cycles
```

---

## Feature Comparison

| Feature | v0.2 | v0.3 |
|---------|------|------|
| Type annotations | ❌ | ✅ Optional |
| Type checking | ❌ | ✅ `izi check` |
| Classes | ❌ | ✅ Minimal OOP |
| Inheritance | ❌ | ❌ (v0.4+) |
| GC | Reference counting | Mark-and-sweep |
| Cyclic structures | ❌ Leak | ✅ Supported |
| Stack traces | Basic | Complete |
| VM imports | ❌ Broken | ✅ Fixed |
| Semantic analysis | ❌ | ✅ Warnings |

---

## Common Pitfalls

### 1. Type Annotation Syntax

```izi
// WRONG: Type after equals
var x = 10: Number;

// CORRECT: Type after variable name
var x: Number = 10;
```

### 2. `this` in Functions

```izi
// WRONG: 'this' in regular function
fn process() {
    print(this.value);  // ERROR
}

// CORRECT: 'this' only in methods
class Data {
    var value: Number;
    fn process() {
        print(this.value);  // OK
    }
}
```

### 3. Constructor Name

```izi
// WRONG: Any method name
class Point {
    fn init(x, y) { ... }  // Won't be called as constructor
}

// CORRECT: Must be named 'constructor'
class Point {
    fn constructor(x, y) { ... }
}
```

---

## Performance Considerations

### GC Overhead

The new GC may introduce **brief pauses** during collection. For most applications, this is negligible. To tune:

```bash
# Increase GC threshold (less frequent collections)
export IZI_GC_THRESHOLD=2097152  # 2 MB

# Monitor GC behavior
izi run --gc-stats yourapp.iz
```

### VM vs Interpreter

The VM is now production-ready and **faster** than the interpreter:

```bash
# Force VM (now default for most cases)
izi run yourapp.iz

# Force interpreter (for debugging)
izi run --interp yourapp.iz
```

---

## Deprecation Warnings

### None in v0.3!

IziLang v0.3 introduces **no deprecations**. All v0.2 features remain fully supported.

---

## Gradual Adoption Strategy

You don't need to migrate everything at once. We recommend:

1. **Week 1**: Run `izi check` on your codebase
2. **Week 2**: Fix warnings (unused variables, dead code)
3. **Week 3**: Add type annotations to public APIs
4. **Week 4**: Convert key objects to classes

Take your time — there's no rush!

---

## Testing Your Migration

### Before Committing Changes

```bash
# 1. Run existing test suite
izi test

# 2. Run semantic analysis
izi check src/**/*.iz

# 3. Check type safety (with --strict)
izi check --strict src/**/*.iz

# 4. Verify runtime behavior
izi run --gc-stats yourapp.iz
```

### Regression Testing

Compare v0.2 and v0.3 behavior:

```bash
# Run with v0.2 (if you have it)
/path/to/v0.2/izi run test.iz > v02.txt

# Run with v0.3
izi run test.iz > v03.txt

# Compare outputs
diff v02.txt v03.txt
```

---

## Getting Help

### Resources

- [v0.3 Specification](V03_SPECIFICATION.md)
- [API Reference](API_REFERENCE.md)
- [GitHub Issues](https://github.com/ismobaga/izilang/issues)

### Common Questions

**Q: Do I have to add types?**  
A: No! Type annotations are completely optional.

**Q: Will my v0.2 code break?**  
A: No. v0.3 is 100% backward compatible.

**Q: Should I use classes or maps?**  
A: Either works. Classes offer better syntax and future optimizations.

**Q: Is the VM stable?**  
A: Yes! The VM is now production-ready.

**Q: Will GC slow down my code?**  
A: For most apps, no. GC pauses are typically < 1ms.

---

## Example Migration

See [examples/migration_v02_v03.iz](../examples/migration_v02_v03.iz) for a complete before/after example.

---

**Migration Complete?** 🎉

Welcome to IziLang v0.3! Enjoy the new features.

---

**Last Updated**: 2026-02-11  
**Version**: 0.3.0

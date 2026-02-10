# IziLang Suggested Features

This document outlines new features and improvements that could be implemented in IziLang.

---

## 🚀 Next Steps (Immediate)

### Step 1: Fix Critical Build Issues
**Priority**: P0 - Required to proceed  
**Effort**: 2-4 hours

1. **Fix vm.cpp syntax error** (line 40)
   - Change `static_cast<OpCode>() readByte());` to `static_cast<OpCode>(readByte());`
   
2. **Remove duplicate declarations in vm.hpp**
   - Remove lines 26-27 (public push/pop)
   - Keep only private declarations (lines 38-39)

3. **Add Chunk::addName() method**
   - Add `std::vector<std::string> names;` to Chunk struct
   - Implement `size_t addName(const std::string& name)` method

4. **Implement VM stack operations**
   - Implement `VM::push(Value v)`
   - Implement `VM::pop()`
   - Implement `VM::binaryNumeric()` template

**Success Criteria**: `make config=debug` completes without errors.

---

### Step 2: Complete Core Bytecode VM
**Priority**: P1 - Make VM functional  
**Effort**: 8-12 hours

1. **Add collection creation opcodes**
   - `CREATE_ARRAY` - Pop N values, create array
   - `CREATE_MAP` - Pop N key-value pairs, create map
   - Implement in `vm.cpp` switch statement

2. **Complete index operations**
   - Fix INDEX opcode with bounds checking
   - Add SET_INDEX opcode for `arr[i] = value`
   - Support both arrays and maps

3. **Implement logic operators**
   - Add `AND` and `OR` opcodes with short-circuit evaluation
   - Update parser to handle `and`/`or` keywords
   - Create LogicalExpr AST node

4. **Add local variable support**
   - Implement scope analysis in compiler
   - Emit GET_LOCAL/SET_LOCAL opcodes
   - Track stack frame offsets

**Success Criteria**: Arrays, maps, and basic logic work in VM mode.

---

### Step 3: Achieve Feature Parity
**Priority**: P1 - VM matches interpreter  
**Effort**: 12-16 hours

1. **Implement function compilation**
   - Compile function bodies to separate chunks
   - Implement CALL opcode with call frame management
   - Handle return values properly
   - Support closures (capture environment)

2. **Implement import compilation**
   - Parse and compile imported modules
   - Link module namespaces
   - Handle circular dependencies

3. **Add control flow enhancements**
   - Implement `break` statement
   - Implement `continue` statement
   - Update while/for loop compilation to handle jumps

**Success Criteria**: `test.iz` runs identically in both interpreter and VM modes.

---

## 🎯 Short-Term Features (0-3 months)

### Feature 1: Standard Library
**Priority**: P1  
**Effort**: 20-30 hours

**Core Modules**:
- `math` - Mathematical functions (sin, cos, sqrt, etc.)
- `string` - String manipulation (split, join, trim, etc.)
- `array` - Array utilities (map, filter, reduce, etc.)
- `io` - File I/O operations (read, write, exists, etc.)
- `json` - JSON parsing and generation
- `http` - Basic HTTP client

**Implementation Approach**:
1. Create native function registration system
2. Implement each module in C++
3. Auto-import into global scope or require explicit import
4. Document all functions

**Example**:
```javascript
import "math";
var pi = math.pi;
var result = math.sqrt(16);  // 4

import "array";
var nums = [1, 2, 3, 4, 5];
var doubled = array.map(nums, fn(x) { return x * 2; });
```

---

### Feature 2: Error Handling
**Priority**: P1  
**Effort**: 15-20 hours

**Components**:
1. **Try-Catch-Finally**
   ```javascript
   try {
       var result = riskyOperation();
   } catch (e) {
       print("Error:", e);
   } finally {
       cleanup();
   }
   ```

2. **Throw Statement**
   ```javascript
   fn divide(a, b) {
       if (b == 0) {
           throw "Division by zero";
       }
       return a / b;
   }
   ```

3. **Stack Traces**
   - Track call stack with file/line information
   - Pretty-print on uncaught exceptions

**Implementation**:
- Add TryStmt, CatchStmt, ThrowStmt to AST
- Implement exception unwinding in interpreter
- Add TRY/CATCH opcodes for VM with jump tables

---

### Feature 3: Better Collections
**Priority**: P2  
**Effort**: 10-15 hours

**Array Enhancements**:
- `push()`, `pop()`, `shift()`, `unshift()` methods
- `slice()`, `splice()` for sub-arrays
- Array destructuring: `var [a, b] = [1, 2];`
- Spread operator: `var all = [...arr1, ...arr2];`

**Map Enhancements**:
- `keys()`, `values()`, `entries()` iterators
- `has()`, `delete()` methods
- Map destructuring: `var {name, age} = person;`
- Spread operator: `var merged = {...obj1, ...obj2};`

**Set Type**:
- New collection type for unique values
- `add()`, `has()`, `delete()`, `size` operations

---

### Feature 4: First-Class Functions
**Priority**: P2  
**Effort**: 8-12 hours

**Features**:
1. **Higher-Order Functions**
   ```javascript
   fn map(arr, fn) {
       var result = [];
       for (var i = 0; i < len(arr); i = i + 1) {
           result[i] = fn(arr[i]);
       }
       return result;
   }
   ```

2. **Anonymous Functions**
   ```javascript
   var add = fn(a, b) { return a + b; };
   ```

3. **Closures**
   ```javascript
   fn makeCounter() {
       var count = 0;
       return fn() {
           count = count + 1;
           return count;
       };
   }
   var counter = makeCounter();
   print(counter());  // 1
   print(counter());  // 2
   ```

**Implementation**:
- Store environment pointer in function objects
- Implement upvalue capture mechanism
- Handle variable resolution across scopes

---

### Feature 5: String Interpolation
**Priority**: P2  
**Effort**: 5-8 hours

**Syntax**:
```javascript
var name = "Alice";
var age = 30;
var message = "Hello, ${name}! You are ${age} years old.";
print(message);  // "Hello, Alice! You are 30 years old."
```

**Implementation**:
- Extend lexer to recognize `${}` in strings
- Parse expression inside interpolation
- Compile to string concatenation operations

---

### Feature 6: Better CLI
**Priority**: P2  
**Effort**: 3-5 hours

**Features**:
- `--vm` or `--interpreter` flag to choose execution mode
- `--version` to show version info
- `--help` for usage information
- `-c "code"` to execute inline code
- `--debug` for verbose output
- REPL mode (interactive prompt)

**Example**:
```bash
izi --version               # IziLang 0.1.0
izi --help                  # Show usage
izi --vm script.iz          # Run with bytecode VM
izi --interpreter script.iz # Run with tree-walker
izi -c "print(1 + 2)"       # Execute inline
izi                         # Enter REPL
```

---

## 🌟 Medium-Term Features (3-6 months)

### Feature 7: Object-Oriented Programming
**Priority**: P2  
**Effort**: 40-50 hours

**Classes**:
```javascript
class Animal {
    fn init(name) {
        this.name = name;
    }
    
    fn speak() {
        print(this.name, "makes a sound");
    }
}

class Dog extends Animal {
    fn speak() {
        print(this.name, "barks");
    }
}

var dog = Dog("Buddy");
dog.speak();  // "Buddy barks"
```

**Features**:
- Class definitions with methods
- Constructor (`init` method)
- Inheritance with `extends`
- `this` keyword for instance access
- `super` for parent class access
- Method overriding

---

### Feature 8: Type System (Optional)
**Priority**: P3  
**Effort**: 60-80 hours

**Gradual Typing**:
```javascript
fn add(a: number, b: number): number {
    return a + b;
}

var name: string = "Alice";
var age: number = 30;

class Person {
    var name: string;
    var age: number;
    
    fn greet(): string {
        return "Hello, " + this.name;
    }
}
```

**Benefits**:
- Catch type errors at compile time
- Better IDE support (autocomplete, refactoring)
- Performance optimizations (monomorphization)

**Implementation**:
- Type inference algorithm
- Type checking pass after parsing
- Optional type annotations (backward compatible)

---

### Feature 9: Module System
**Priority**: P2  
**Effort**: 30-40 hours

**Export/Import**:
```javascript
// math.iz
export fn add(a, b) { return a + b; }
export fn subtract(a, b) { return a - b; }
export var PI = 3.14159;

// main.iz
import { add, PI } from "math.iz";
import * as math from "math.iz";

print(add(1, 2));        // 3
print(math.subtract(5, 3)); // 2
```

**Features**:
- Named exports
- Default exports
- Wildcard imports
- Module resolution (relative and absolute paths)
- Circular dependency handling

---

### Feature 10: Async/Await
**Priority**: P3  
**Effort**: 50-70 hours

**Syntax**:
```javascript
async fn fetchData(url) {
    var response = await http.get(url);
    return response.json();
}

async fn main() {
    var data = await fetchData("https://api.example.com/data");
    print(data);
}

main();
```

**Implementation**:
- Transform async functions to state machines
- Implement Promise/Future type
- Event loop for async execution
- Native async I/O functions

---

## 🔬 Advanced Features (6-12 months)

### Feature 11: Pattern Matching
**Priority**: P3  
**Effort**: 30-40 hours

**Syntax**:
```javascript
fn describe(value) {
    match value {
        0 => "zero",
        1 => "one",
        x if x > 0 => "positive",
        x if x < 0 => "negative",
        _ => "unknown"
    }
}
```

---

### Feature 12: Generics
**Priority**: P3  
**Effort**: 50-60 hours

**Syntax**:
```javascript
fn identity<T>(value: T): T {
    return value;
}

class Box<T> {
    var value: T;
    
    fn init(v: T) {
        this.value = v;
    }
}
```

---

### Feature 13: Macros/Metaprogramming
**Priority**: P3  
**Effort**: 60-80 hours

**Compile-Time Evaluation**:
```javascript
@compile_time
fn generate_getter(fieldName) {
    return "fn get${fieldName}() { return this.${fieldName}; }";
}
```

---

### Feature 14: JIT Compilation
**Priority**: P4  
**Effort**: 100-150 hours

**Approach**:
- Profile hotspots at runtime
- Generate machine code for hot functions
- Use LLVM or custom JIT backend

---

## 📊 Feature Priority Matrix

| Feature | Priority | Effort | User Impact | Complexity |
|---------|----------|--------|-------------|------------|
| Fix Build Issues | P0 | 4h | Critical | Low |
| Complete VM | P1 | 12h | High | Medium |
| Standard Library | P1 | 30h | High | Medium |
| Error Handling | P1 | 20h | High | Medium |
| Better CLI | P2 | 5h | Medium | Low |
| Collections | P2 | 15h | High | Medium |
| First-Class Functions | P2 | 12h | High | Medium |
| String Interpolation | P2 | 8h | Medium | Low |
| Module System | P2 | 40h | High | High |
| OOP | P2 | 50h | High | High |
| Type System | P3 | 80h | Medium | Very High |
| Async/Await | P3 | 70h | High | Very High |
| Pattern Matching | P3 | 40h | Medium | High |
| Generics | P3 | 60h | Medium | Very High |
| Macros | P3 | 80h | Low | Very High |
| JIT | P4 | 150h | High | Extreme |

---

## 🎓 Learning Projects

For contributors looking to learn, these are good starter projects:

### Beginner
1. Add `--version` flag (2-3 hours)
2. Remove unused includes/fields (1-2 hours)
3. Add division by zero check (1 hour)
4. Write test suite basics (3-5 hours)

### Intermediate
1. Implement CREATE_ARRAY opcode (4-6 hours)
2. Add string interpolation (8-10 hours)
3. Implement break/continue (6-8 hours)
4. Add standard library math module (8-10 hours)

### Advanced
1. Implement closures (15-20 hours)
2. Add try-catch error handling (20-25 hours)
3. Implement OOP classes (40-50 hours)
4. Build module system (30-40 hours)

---

## 🗺️ Roadmap Timeline

### Q1 2026: Foundation
- ✅ Fix all critical build issues
- ✅ Complete bytecode VM core
- ✅ Achieve feature parity between VM and interpreter
- ✅ Add comprehensive test suite
- ✅ Improve documentation

### Q2 2026: Expansion
- 🎯 Implement standard library
- 🎯 Add error handling (try-catch)
- 🎯 Improve collections (arrays, maps, sets)
- 🎯 Better CLI with REPL
- 🎯 Module system basics

### Q3 2026: Enhancement
- 🎯 Object-oriented programming
- 🎯 First-class functions and closures
- 🎯 Async/await support
- 🎯 Performance optimizations

### Q4 2026: Polish
- 🎯 Optional type system
- 🎯 Advanced features (pattern matching, generics)
- 🎯 Developer tools (debugger, profiler)
- 🎯 Production readiness

---

## Contributing

To contribute a new feature:

1. **Discuss**: Open an issue describing the feature
2. **Design**: Document the syntax and semantics
3. **Implement**: Start with interpreter, then add VM support
4. **Test**: Add comprehensive test cases
5. **Document**: Update language guide and examples
6. **Review**: Submit PR for code review

See `CONTRIBUTING.md` for detailed guidelines.

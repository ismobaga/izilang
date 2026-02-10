# IziLang Codebase Analysis

**Date**: 2026-02-10  
**Version**: Analysis of commit `ecf3dd5`  
**Total Lines of Code**: ~2,692 lines (C++ source)

---

## Executive Summary

IziLang is a programming language interpreter implemented in C++20, featuring:
- **Dual execution modes**: Tree-walking interpreter and bytecode VM
- **Core features**: Functions, variables, control flow, arrays, maps
- **Architecture**: Visitor pattern for AST traversal, std::variant for runtime values
- **Status**: ⚠️ **DOES NOT COMPILE** - Critical errors prevent build

### Critical Status
- ❌ **Compilation**: FAILS with 3 critical errors
- ❌ **Bytecode VM**: Incomplete implementation
- ✅ **Tree-walker Interpreter**: Mostly functional
- ⚠️ **Test Coverage**: Minimal (only manual test.iz file)

---

## 1. Critical Issues Blocking Compilation

### 1.1 Syntax Error in VM (vm.cpp:40)

**File**: `src/bytecode/vm.cpp` line 40  
**Severity**: 🔴 **CRITICAL** - Blocks all compilation

```cpp
// CURRENT (BROKEN):
OpCode op = static_cast<OpCode>() readByte());  // Invalid syntax

// SHOULD BE:
OpCode op = static_cast<OpCode>(readByte());
```

**Impact**: Entire project fails to compile.

---

### 1.2 Duplicate Method Declarations in VM (vm.hpp)

**File**: `src/bytecode/vm.hpp` lines 26-27, 38-39  
**Severity**: 🔴 **CRITICAL** - Compilation error

```cpp
public:
    void push(Value value);  // Line 26
    Value pop();             // Line 27
    
private:
    void push(Value v);      // Line 38 - DUPLICATE!
    Value pop();             // Line 39 - DUPLICATE!
```

**Issue**: C++ doesn't allow method overloading with identical signatures.  
**Fix**: Remove public declarations (lines 26-27), keep only private versions.

---

### 1.3 Missing Chunk::addName() Method

**File**: `src/bytecode/chunk.hpp`  
**Severity**: 🔴 **CRITICAL** - Linker errors

**Issue**: Compiler code calls `chunk.addName()` but method doesn't exist:
- `compiler.cpp:25-30` - Variable declarations
- `compiler.cpp:159` - Variable assignments  
- `compiler.cpp:166` - Variable reads
- `compiler.cpp:268` - Function calls

**Required Addition**:
```cpp
struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<std::string> names;  // ADD THIS
    
    size_t addName(const std::string& name) {  // ADD THIS
        names.push_back(name);
        return names.size() - 1;
    }
};
```

---

## 2. Incomplete Implementations

### 2.1 Function Compilation Missing

**File**: `src/compile/compiler.cpp` lines 273-276  
**Severity**: 🟠 **HIGH** - Major feature gap

```cpp
void BytecodeCompiler::visit(FunctionStmt& stmt) {
    // Function compilation not implemented yet
    throw std::runtime_error("Function compilation not implemented.");
}
```

**Impact**: Cannot use functions with bytecode VM. Code like this crashes:
```javascript
fn add(a, b) {
    return a + b;
}
```

**Status**: Declared in AST, works in interpreter, but not in bytecode compiler.

---

### 2.2 Import Statement Compilation Missing

**File**: `src/compile/compiler.cpp` lines 278-281  
**Severity**: 🟠 **HIGH** - Major feature gap

```cpp
void BytecodeCompiler::visit(ImportStmt& stmt) {
    // Import compilation not implemented yet
    throw std::runtime_error("Import compilation not implemented.");
}
```

**Impact**: Cannot use module imports with bytecode VM.

---

### 2.3 Missing Array/Map Opcodes

**File**: `src/bytecode/opcode.hpp`  
**Severity**: 🟠 **HIGH** - Collections broken in VM

**Missing Opcodes**:
- `CREATE_ARRAY` - Build array from stack values
- `CREATE_MAP` - Build map from stack key-value pairs
- `GET_INDEX` - Array/map element access (INDEX exists but incomplete)
- `SET_INDEX` - Array/map element assignment

**Current Workaround**: Compiler partially implements (lines 189-208) but VM has no corresponding execution.

---

### 2.4 Logic Operators Not Implemented

**File**: `src/bytecode/opcode.hpp` + compiler  
**Severity**: 🟡 **MEDIUM** - Feature gap

**Missing**: 
- `AND` opcode for `&&` operator
- `OR` opcode for `||` operator

**Status**: 
- Lexer recognizes `AND` and `OR` keywords
- Parser doesn't handle them
- AST has no logical operator nodes
- Bytecode VM has no implementation

---

## 3. Architecture Analysis

### 3.1 Dual Execution Modes

**Location**: `src/main.cpp` lines 36-51

```cpp
bool useVM = true;  // Toggle between interpreter and bytecode VM

if (!useVM) {
    Interpreter interp;
    interp.interpret(program);
} else {
    BytecodeCompiler compiler;
    Chunk chunk = compiler.compile(program);
    VM vm;
    Value result = vm.run(chunk);
}
```

**Design Assessment**:
- ✅ **Pros**: Flexibility, can test both implementations
- ❌ **Cons**: 
  - Duplicate maintenance burden
  - Feature parity issues (functions work in interpreter, not VM)
  - No clear migration strategy
  - useVM hardcoded, should be CLI flag

**Recommendation**: Pick one execution model or maintain feature parity.

---

### 3.2 Visitor Pattern Implementation

**Quality**: ✅ **Well-Designed**

**Structure**:
- `Expr` base class with `accept(ExprVisitor&)` virtual method
- `Stmt` base class with `accept(StmtVisitor&)` virtual method  
- Concrete AST nodes implement `accept()` with double-dispatch
- Visitors (Interpreter, Compiler) implement `visit()` for each node type

**Example**:
```cpp
struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    
    Value accept(ExprVisitor& v) override { 
        return v.visit(*this);  // Double dispatch
    }
};
```

**Assessment**: Industry-standard implementation, clean separation of concerns.

---

### 3.3 Runtime Value System

**Location**: `src/common/value.hpp`

```cpp
using Value = std::variant<
    Nil,
    bool,
    double,
    std::string,
    std::shared_ptr<Array>,
    std::shared_ptr<Map>,
    std::shared_ptr<Callable>,
    std::shared_ptr<VmCallable>
>;
```

**Quality**: ✅ **Modern C++20 Design**

**Pros**:
- Type-safe runtime values
- No manual type tags needed
- `std::holds_alternative` for type checking
- `std::get` for value extraction

**Cons**:
- Shared pointers for all composite types (slight overhead)
- Circular dependency with Callable (resolved via forward declarations)

---

## 4. Security & Bug Analysis

### 4.1 Critical Bug: SetIndexExpr Bounds Check

**File**: `src/interp/interpreter.cpp` line 207  
**Severity**: 🟠 **HIGH** - Logic error

```cpp
size_t idx = static_cast<size_t>(std::get<double>(indexVal));
if (idx < 0) {  // BUG: size_t is unsigned, condition always false!
    throw std::runtime_error("Array index out of bounds.");
}
```

**Issue**: `size_t` is unsigned, so `idx < 0` is always false. Check does nothing.

**Fix**:
```cpp
double indexDouble = std::get<double>(indexVal);
if (indexDouble < 0 || indexDouble >= arr->elements.size()) {
    throw std::runtime_error("Array index out of bounds.");
}
size_t idx = static_cast<size_t>(indexDouble);
```

---

### 4.2 Division by Zero Not Checked

**Files**: `src/interp/interpreter.cpp`, `src/bytecode/vm.cpp`  
**Severity**: 🟡 **MEDIUM** - Runtime crash possible

**Locations**:
- Interpreter: Line 83 `return asNumber(left) / asNumber(right);`
- Compiler: Line 76, 84
- VM: OpCode::DIVIDE case

**Risk**: Program crashes on `x / 0` instead of graceful error.

**Fix**: Add validation before division.

---

### 4.3 Module Path Security

**File**: `src/interp/interpreter.cpp` lines 295-301  
**Severity**: 🟡 **MEDIUM** - Potential security issue

```cpp
std::string Interpreter::normalizeModulePath(const std::string& modulePath) {
    if (modulePath.rfind("./", 0) == 0) {
        return modulePath.substr(2);  // Remove "./"
    }
    return modulePath;
}
```

**Issues**:
- No validation against path traversal (`../../etc/passwd`)
- Doesn't check if path exists
- No sandboxing of import paths

**Recommendation**: Use `std::filesystem::canonical()` or restrict to specific directories.

---

### 4.4 Memory Management Concerns

**File**: `src/interp/environment.hpp`  
**Severity**: 🟡 **MEDIUM** - Potential dangling pointer

```cpp
class Environment {
    Environment* parent;  // Raw pointer - no ownership semantics!
    // ...
};
```

**Risk**: If parent environment is deleted, this becomes a dangling pointer.

**Mitigation**: Current code structure likely safe (parent outlives children), but fragile design.

**Better**: Use `std::shared_ptr<Environment>` or document ownership clearly.

---

## 5. Missing Features & Gaps

### 5.1 Unused Keywords

**File**: `src/common/token.hpp`  
**Status**: Lexer recognizes but no implementation

| Keyword | Token | Usage |
|---------|-------|-------|
| `break` | BREAK | No parser/AST support |
| `continue` | CONTINUE | No parser/AST support |
| `class` | CLASS | No OOP implementation |
| `super` | SUPER | No inheritance |
| `this` | THIS | No method context |
| `and` | AND | Not in parser |
| `or` | OR | Not in parser |

**Recommendation**: Remove from lexer or implement features.

---

### 5.2 Unused AST Fields

**File**: `src/ast/expr.hpp` line 33

```cpp
struct VariableExpr : Expr {
    std::string name;
    ExprPtr value;  // NEVER USED
    // ...
};
```

**Issue**: Constructor takes `value` parameter but it's never accessed anywhere in codebase.

**Recommendation**: Remove or document intended usage.

---

### 5.3 Local Variables in Bytecode VM

**File**: `src/bytecode/opcode.hpp`  
**Status**: Opcodes exist but no compilation support

```cpp
enum class OpCode : uint8_t {
    // ...
    GET_LOCAL,    // Declared but compiler never emits
    SET_LOCAL,    // Declared but compiler never emits
    // ...
};
```

**Impact**: All variables go through global scope in VM (slow, incorrect scoping).

---

## 6. Code Quality Issues

### 6.1 Unused Includes

**File**: `src/common/value.hpp` line 3

```cpp
#include <future>  // NEVER USED
```

**Impact**: Increases compilation time, adds bloat.

---

### 6.2 Magic Numbers

**Examples**:
- `vm.hpp:10` - `constexpr size_t STACK_MAX = 256;` (why 256?)
- `vm.cpp:10` - `frames.reserve(64);` (why 64?)

**Recommendation**: Document choices or make configurable.

---

### 6.3 Commented-Out Code

**File**: `src/bytecode/vm.cpp` line 147

```cpp
// ... handle other opcodes ...
```

**Status**: Placeholder comment with no corresponding code.

---

## 7. Testing Status

### 7.1 Current Tests

**Files**:
- `test.iz` - Manual integration test
- `mod.iz` - Simple module test

**Content** (test.iz):
```javascript
import "mod.iz";
var x = 10;
fn sumArray(a) { /* ... */ }
var ab = [1,2,3];
print(sumArray(ab));
```

**Issues**:
- No unit tests
- No automated test runner
- No test framework integration (gtest, catch2, etc.)
- Can't verify bytecode VM (functions not implemented)

---

### 7.2 Missing Test Coverage

**Untested Areas**:
- Bytecode compiler correctness
- VM instruction execution
- Error handling paths
- Edge cases (division by zero, index out of bounds)
- Module import resolution
- Function calls and closures

---

## 8. Build System

### 8.1 Premake Configuration

**File**: `premake5.lua`  
**Status**: ✅ **Well-Configured**

```lua
workspace "IziLang"
configurations {"Debug", "Release"}
cppdialect "c++20"
```

**Positives**:
- C++20 enabled
- Debug and Release configurations
- Platform-specific linking (pthread, dl, m on Linux)

**Suggestions**:
- Add test project
- Enable warnings (`warnings "Extra"`)
- Add static analysis target

---

## 9. Documentation Status

### 9.1 Current Documentation

**Files**:
- `README.md` - Empty
- `.github/copilot-instructions.md` - Architecture notes

**Grade**: ❌ **INSUFFICIENT**

### 9.2 Missing Documentation

- Build instructions
- Language syntax guide
- Feature list
- API reference
- Examples directory
- Contributing guidelines
- Changelog

---

## 10. Recommended Action Plan

### Phase 1: Fix Critical Build Issues (Priority 1)
1. Fix vm.cpp:40 syntax error
2. Remove duplicate push/pop declarations in vm.hpp
3. Add Chunk::addName() method and names vector
4. Implement VM::push(), VM::pop(), VM::binaryNumeric()
5. **Verify**: Project compiles successfully

### Phase 2: Complete Bytecode VM (Priority 2)
1. Add CREATE_ARRAY, CREATE_MAP, AND, OR opcodes
2. Implement function compilation in BytecodeCompiler
3. Implement array/map operations in VM
4. Add local variable support (GET_LOCAL, SET_LOCAL)
5. Implement import statement compilation
6. **Verify**: test.iz runs on both interpreter and VM

### Phase 3: Security & Bug Fixes (Priority 3)
1. Fix SetIndexExpr bounds check bug
2. Add division by zero validation
3. Improve module path security
4. Add comprehensive error messages
5. **Verify**: Edge cases handled gracefully

### Phase 4: Code Quality (Priority 4)
1. Remove unused includes, fields, keywords
2. Add comprehensive test suite
3. Document all public APIs
4. Add CLI argument parsing (--vm flag, --help, etc.)
5. Create examples directory
6. **Verify**: Code review checklist passes

### Phase 5: Documentation (Priority 5)
1. Write comprehensive README
2. Create language syntax guide
3. Add API documentation
4. Write contributor guide
5. **Verify**: New contributors can onboard

---

## 11. Feature Roadmap

### Implemented Features ✅
- [x] Basic arithmetic (+, -, *, /)
- [x] Comparison operators (==, !=, <, >, <=, >=)
- [x] Variables (var x = value)
- [x] If/else statements
- [x] While loops
- [x] Functions (interpreter only)
- [x] Arrays
- [x] Maps
- [x] String concatenation
- [x] Print statements
- [x] Module imports (interpreter only)
- [x] Native functions (len, clock, etc.)

### Partially Implemented ⚠️
- [~] Functions (works in interpreter, not bytecode VM)
- [~] Imports (works in interpreter, not bytecode VM)
- [~] Arrays/Maps (works in interpreter, incomplete in VM)
- [~] For loops (desugared to while, not used)

### Not Implemented ❌
- [ ] Logical operators (and, or)
- [ ] Break/continue statements
- [ ] Classes and OOP
- [ ] Closures
- [ ] First-class functions
- [ ] Standard library
- [ ] Error handling (try/catch)
- [ ] Type annotations
- [ ] Module system with namespaces

---

## Conclusion

IziLang shows promise as a learning/hobby language project with good architectural foundations (Visitor pattern, modern C++ usage). However, it currently **does not compile** due to critical errors and has a **significant feature gap** between the interpreter and bytecode VM.

**Immediate Priority**: Fix compilation errors to make the project buildable.

**Strategic Decision Needed**: Choose to fully support either:
1. **Tree-walking interpreter only** (simpler, fully functional)
2. **Bytecode VM only** (faster, needs completion)
3. **Both with feature parity** (more work, best long-term)

**Estimated Effort**:
- Phase 1 (Critical fixes): 2-4 hours
- Phase 2 (VM completion): 8-16 hours
- Phase 3 (Security): 4-8 hours
- Phase 4 (Quality): 8-12 hours
- Phase 5 (Docs): 4-6 hours

**Total**: ~26-46 hours for full completion.

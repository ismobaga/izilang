# IziLang Problems & Issues

This document lists all identified problems in the IziLang codebase, organized by severity.

---

## 🔴 Critical (Blocks Compilation)

### Problem 1: Syntax Error in VM
- **File**: `src/bytecode/vm.cpp` line 40
- **Description**: Invalid C++ syntax prevents compilation
- **Current Code**: `OpCode op = static_cast<OpCode>() readByte());`
- **Expected Code**: `OpCode op = static_cast<OpCode>(readByte());`
- **Impact**: Entire project fails to compile
- **Fix Difficulty**: Trivial (1 line change)

### Problem 2: Duplicate Method Declarations in VM
- **File**: `src/bytecode/vm.hpp` lines 26-27 and 38-39
- **Description**: `push()` and `pop()` declared twice with identical signatures
- **Impact**: Compilation error - cannot overload with same signature
- **Fix Difficulty**: Easy (remove 2 lines)

### Problem 3: Missing Chunk::addName() Method
- **File**: `src/bytecode/chunk.hpp`
- **Description**: Compiler calls `chunk.addName()` but method doesn't exist
- **Referenced in**: 
  - `compiler.cpp:25-30` (variable declarations)
  - `compiler.cpp:159` (assignments)
  - `compiler.cpp:166` (variable reads)
  - `compiler.cpp:268` (function calls)
- **Impact**: Linker error, compiler cannot build
- **Fix Difficulty**: Easy (add method + vector)

### Problem 4: Missing VM Method Implementations
- **File**: `src/bytecode/vm.hpp` declares but `vm.cpp` doesn't implement
- **Missing Methods**:
  - `void push(Value v)`
  - `Value pop()`
  - `void binaryNumeric(Fn fn)` template
- **Impact**: Linker errors when trying to build VM
- **Fix Difficulty**: Medium (3 method implementations)

---

## 🟠 High Priority (Major Feature Gaps)

### Problem 5: Function Compilation Not Implemented
- **File**: `src/compile/compiler.cpp` lines 273-276
- **Description**: Throws exception when encountering function declarations
- **Impact**: Functions work in interpreter but crash in bytecode VM
- **Test Case**: 
  ```javascript
  fn add(a, b) { return a + b; }  // Crashes VM
  ```
- **Fix Difficulty**: Hard (requires call frame management, scope tracking)

### Problem 6: Import Compilation Not Implemented
- **File**: `src/compile/compiler.cpp` lines 278-281
- **Description**: Throws exception when encountering import statements
- **Impact**: Module system doesn't work in bytecode VM
- **Test Case**:
  ```javascript
  import "mod.iz";  // Crashes VM
  ```
- **Fix Difficulty**: Hard (requires module linking, name resolution)

### Problem 7: Array/Map Creation Opcodes Missing
- **File**: `src/bytecode/opcode.hpp` and `vm.cpp`
- **Description**: Compiler partially generates array/map code but opcodes don't exist
- **Missing Opcodes**:
  - `CREATE_ARRAY` - Build array from N stack values
  - `CREATE_MAP` - Build map from key-value pairs on stack
- **Impact**: Arrays and maps don't work in bytecode VM
- **Fix Difficulty**: Medium (2 new opcodes + VM execution logic)

### Problem 8: Index Access Incomplete in VM
- **File**: `src/bytecode/vm.cpp`
- **Description**: `INDEX` opcode exists but incomplete implementation
- **Issues**:
  - No bounds checking
  - No error handling for invalid types
  - SET_INDEX_EXPR not handled at all
- **Fix Difficulty**: Medium (complete INDEX, add SET_INDEX opcode)

---

## 🟡 Medium Priority (Functionality Issues)

### Problem 9: Logic Operators Not Supported
- **Description**: `and` and `or` keywords recognized but not implemented
- **Files**:
  - Lexer recognizes `AND` and `OR` tokens
  - Parser doesn't handle them
  - No AST nodes for logical operators
  - No bytecode opcodes for short-circuit evaluation
- **Impact**: Cannot use `if (x and y)` or `if (a or b)`
- **Fix Difficulty**: Medium (parser + AST + compiler + VM)

### Problem 10: Local Variables Not Supported in VM
- **File**: `src/bytecode/opcode.hpp` and compiler
- **Description**: `GET_LOCAL` and `SET_LOCAL` opcodes exist but never emitted
- **Impact**: All variables use global scope in VM (slow, incorrect scoping)
- **Example Problem**:
  ```javascript
  fn outer() {
      var x = 1;
      fn inner() {
          var x = 2;  // Should be local but acts global
          return x;
      }
      return inner() + x;  // Incorrect result
  }
  ```
- **Fix Difficulty**: Hard (requires scope analysis, stack frame offsets)

### Problem 11: SetIndexExpr Bounds Check Bug
- **File**: `src/interp/interpreter.cpp` line 207
- **Description**: Bounds check uses `idx < 0` on unsigned `size_t`, always false
- **Current Code**:
  ```cpp
  size_t idx = static_cast<size_t>(std::get<double>(indexVal));
  if (idx < 0) {  // BUG: size_t is unsigned!
      throw std::runtime_error("Array index out of bounds.");
  }
  ```
- **Impact**: Negative indices not caught, potential crash or undefined behavior
- **Fix Difficulty**: Easy (check before casting)

### Problem 12: Division by Zero Not Checked
- **Files**: Interpreter and VM division operations
- **Description**: No validation before division operations
- **Impact**: Program crashes on `x / 0` instead of throwing exception
- **Locations**:
  - `interpreter.cpp:83` - Binary division
  - `vm.cpp` - OpCode::DIVIDE
  - `compiler.cpp:76, 84` - Division compilation
- **Fix Difficulty**: Easy (add check before operation)

### Problem 13: Module Path Security Issue
- **File**: `src/interp/interpreter.cpp` lines 295-301
- **Description**: Simple string manipulation, no security validation
- **Vulnerability**: Path traversal attack possible
- **Example**: `import "../../etc/passwd"`
- **Fix Difficulty**: Medium (use std::filesystem, sandbox imports)

### Problem 14: For Loop Dead Code
- **File**: `src/parse/parser.cpp` lines 143-192
- **Description**: Parser has `forStatement()` method but never calls it
- **Status**: For loops desugar to while loops in parser
- **Impact**: Code bloat, confusion about feature support
- **Fix Difficulty**: Easy (remove or integrate into statement())

---

## 🟢 Low Priority (Code Quality)

### Problem 15: Unused Include
- **File**: `src/common/value.hpp` line 3
- **Description**: `#include <future>` never used
- **Impact**: Slower compilation
- **Fix Difficulty**: Trivial (remove 1 line)

### Problem 16: Unused VariableExpr::value Field
- **File**: `src/ast/expr.hpp` line 33
- **Description**: `VariableExpr` has `value` member that's never accessed
- **Impact**: Memory waste, confusion about purpose
- **Fix Difficulty**: Easy (remove or document)

### Problem 17: Unused Keywords
- **Files**: `src/common/token.hpp`, `src/parse/lexer.cpp`
- **Description**: Lexer recognizes keywords with no implementation
- **List**:
  - `break` - No control flow support
  - `continue` - No control flow support  
  - `class` - No OOP
  - `super` - No inheritance
  - `this` - No method context
- **Impact**: False expectation of features
- **Fix Difficulty**: Easy (remove from lexer) or Hard (implement features)

### Problem 18: Magic Numbers Without Documentation
- **File**: `src/bytecode/vm.hpp` line 10, `vm.cpp` line 10
- **Examples**:
  - `STACK_MAX = 256` - Why 256?
  - `frames.reserve(64)` - Why 64 call frames?
- **Impact**: Hard to tune performance, unclear constraints
- **Fix Difficulty**: Easy (add comments or make configurable)

### Problem 19: Raw Pointer in Environment
- **File**: `src/interp/environment.hpp`
- **Description**: `Environment* parent` is raw pointer with unclear ownership
- **Risk**: Potential dangling pointer if parent deleted
- **Current Safety**: Likely safe due to stack-based lifecycle
- **Fix Difficulty**: Medium (change to shared_ptr or document contract)

### Problem 20: Dual Execution Mode Complexity
- **File**: `src/main.cpp` lines 36-51
- **Description**: Hardcoded `useVM` flag switches execution modes
- **Issues**:
  - No CLI argument to control mode
  - Feature parity not maintained (functions work in one, not other)
  - Doubles maintenance burden
  - No clear migration path documented
- **Fix Difficulty**: Medium (add CLI flags, document strategy)

---

## 📊 Problem Summary

| Severity | Count | Compilation Impact | Runtime Impact |
|----------|-------|-------------------|----------------|
| 🔴 Critical | 4 | ❌ Blocks build | N/A |
| 🟠 High | 6 | ✅ Compiles (after critical fixes) | ❌ Major features broken |
| 🟡 Medium | 10 | ✅ Compiles | ⚠️ Bugs, missing features |
| 🟢 Low | 6 | ✅ Compiles | ✅ Works but code quality issues |

**Total Problems**: 26

---

## Recommended Fix Order

### Sprint 1: Make It Build (1-2 days)
1. Fix vm.cpp:40 syntax error
2. Remove duplicate push/pop in vm.hpp
3. Implement VM push/pop/binaryNumeric
4. Add Chunk::addName() method
5. **Deliverable**: Project compiles successfully

### Sprint 2: Make Interpreter Robust (2-3 days)
1. Fix SetIndexExpr bounds check bug
2. Add division by zero checks
3. Improve module path security
4. Add comprehensive error messages
5. **Deliverable**: Interpreter handles edge cases gracefully

### Sprint 3: Complete Bytecode VM Basics (5-7 days)
1. Add CREATE_ARRAY and CREATE_MAP opcodes
2. Complete INDEX/SET_INDEX operations
3. Implement local variable support
4. Add logic operators (AND/OR)
5. **Deliverable**: Arrays, maps, logic work in VM

### Sprint 4: Advanced Features (7-10 days)
1. Implement function compilation
2. Implement import compilation
3. Add break/continue support
4. Implement for loops properly
5. **Deliverable**: Feature parity between interpreter and VM

### Sprint 5: Polish (3-5 days)
1. Remove unused code and includes
2. Add comprehensive test suite
3. Improve documentation
4. Add CLI argument parsing
5. **Deliverable**: Production-ready codebase

**Total Estimated Time**: 18-27 days

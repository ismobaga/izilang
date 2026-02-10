# IziLang Codebase Analysis

**Date:** February 10, 2026  
**Repository:** ismobaga/izilang  
**Total Lines of Code:** ~2,719 lines (26 source files)

---

## Executive Summary

IziLang is a programming language interpreter implemented in C++20, featuring:
- **Dual execution modes**: Tree-walking interpreter and bytecode VM compiler
- **Core language features**: Variables, functions, control flow, arrays, maps
- **Modern C++ design**: Uses visitor pattern, `std::variant`, and smart pointers

The project is in **early/mid-development stage** with a solid foundation but several incomplete features and architectural issues that need addressing.

---

## 1. Architecture Overview

### 1.1 Component Structure

```
izilang/
├── src/
│   ├── ast/           # Abstract Syntax Tree nodes
│   │   ├── expr.hpp   # Expression types (Binary, Unary, Literal, etc.)
│   │   ├── stmt.hpp   # Statement types (If, While, Function, etc.)
│   │   └── visitor.hpp # Visitor pattern interfaces
│   ├── common/        # Shared types and utilities
│   │   ├── token.hpp  # Token definitions and types
│   │   ├── value.hpp  # Runtime value representation (std::variant)
│   │   └── callable.hpp # Function interface
│   ├── parse/         # Lexer and Parser
│   │   ├── lexer.cpp/hpp
│   │   └── parser.cpp/hpp
│   ├── interp/        # Tree-walking interpreter
│   │   ├── interpreter.cpp/hpp
│   │   ├── environment.hpp
│   │   ├── user_function.cpp/hpp
│   │   └── native.cpp/hpp
│   ├── compile/       # Bytecode compiler
│   │   └── compiler.cpp/hpp
│   ├── bytecode/      # Virtual Machine
│   │   ├── vm.cpp/hpp
│   │   ├── chunk.hpp
│   │   ├── opcode.hpp
│   │   ├── function.hpp
│   │   └── mv_callable.hpp
│   └── main.cpp       # Entry point
```

### 1.2 Design Patterns Used

1. **Visitor Pattern** for AST traversal
   - Clean separation between AST structure and operations
   - Two visitor types: `ExprVisitor` (returns Value) and `StmtVisitor` (returns void)

2. **std::variant for Runtime Values**
   ```cpp
   using Value = std::variant<
       Nil, bool, double, std::string,
       std::shared_ptr<Array>,
       std::shared_ptr<Map>,
       std::shared_ptr<Callable>,
       std::shared_ptr<VmCallable>
   >;
   ```

3. **Smart Pointers for Memory Management**
   - `std::unique_ptr` for AST node ownership
   - `std::shared_ptr` for runtime objects (functions, arrays, maps)

---

## 2. Current Implementation Status

### 2.1 ✅ Implemented Features

#### Lexer & Parser
- ✅ Token scanning with line/column tracking
- ✅ Comprehensive token types (operators, keywords, literals)
- ✅ Expression parsing (precedence climbing)
- ✅ Statement parsing
- ✅ Error reporting

#### Data Types
- ✅ Primitives: `nil`, `bool`, `double`, `string`
- ✅ Collections: Arrays `[1, 2, 3]`, Maps `{"key": value}`
- ✅ Functions (user-defined and native)

#### Operators
- ✅ Arithmetic: `+`, `-`, `*`, `/`
- ✅ Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
- ✅ Logical: `!`, `and`, `or` (keywords)
- ✅ Unary: `-` (negation), `!` (not)

#### Control Flow
- ✅ If/else statements
- ✅ While loops
- ✅ For loops (parsed, desugared to while)
- ✅ Function calls
- ✅ Return statements
- ✅ Block scoping

#### Advanced Features
- ✅ Array/Map indexing: `arr[0]`, `map["key"]`
- ✅ Index assignment: `arr[0] = 10`
- ✅ Native functions: `print()`, `len()`
- ✅ User-defined functions with parameters
- ✅ Closures support (via environment chain)
- ✅ Import statements (parser only)

#### Execution Modes
- ✅ Tree-walking interpreter (fully functional)
- ✅ Bytecode compiler (partially functional)
- ✅ Stack-based VM (basic implementation)

### 2.2 ❌ Missing/Incomplete Features

#### High Priority
1. **Import System** ⚠️ **CRITICAL**
   - Parser recognizes `import "module.iz"` syntax
   - Interpreter has import loading logic implemented
   - **Compiler throws "Import compilation not implemented"**
   - Currently, VM mode cannot run code with imports

2. **Break/Continue Statements** ⚠️
   - Token types defined: `TokenType::BREAK`, `TokenType::CONTINUE`
   - **Not implemented in parser, interpreter, or compiler**
   - Would require control flow exception mechanism

3. **For Loop Variables** ⚠️
   - For loops are parsed but desugared to while loops
   - Variable scoping might not be correct
   - Needs testing

4. **Class System** ⚠️
   - Token types exist: `CLASS`, `THIS`, `SUPER`
   - **No AST nodes, parser, or interpreter logic**
   - Major feature gap for OOP

#### Medium Priority
5. **Logical Short-Circuit Operators**
   - `AND` and `OR` tokens exist
   - May not have proper short-circuit evaluation
   - Need to verify implementation

6. **Ternary Operator** ⚠️
   - `QUESTION` and `COLON` tokens exist
   - **No parser or interpreter implementation**

7. **Error Handling**
   - No try/catch mechanism
   - No custom exception types for user code
   - Runtime errors just throw C++ exceptions

8. **Module System**
   - Import partially works in interpreter mode
   - No proper module resolution paths
   - No standard library structure

#### Low Priority
9. **Comments Support**
   - Not clear if single-line or multi-line comments work
   - Need to verify lexer implementation

10. **String Escape Sequences**
    - Basic strings work
    - Need to verify escape sequences: `\n`, `\t`, `\\`, etc.

11. **Multi-character Operators**
    - Most are implemented (`==`, `!=`, `<=`, `>=`, `->`)
    - Need to verify arrow operator usage

---

## 3. Problems & Issues

### 3.1 Critical Issues

#### 1. **VM Mode is Unusable with Imports**
**Problem:** The default execution mode is VM (`useVM = true` in main.cpp), but the compiler throws an error for import statements.

**Impact:** Users cannot run the test.iz file with the default settings.

**Solution:**
```cpp
// Option 1: Switch default to interpreter mode
bool useVM = false;

// Option 2: Implement import compilation
// - Add module loading to VM
// - Compile imported modules
// - Merge chunks
```

#### 2. **Build Artifacts in Git**
**Problem:** `bin/` and `obj/` directories were committed despite `.gitignore`.

**Impact:** Repository pollution, merge conflicts, unnecessary size.

**Solution:** Already addressed by excluding them.

#### 3. **No Test Suite**
**Problem:** Only manual testing with `test.iz` and `mod.iz`.

**Impact:** 
- Hard to ensure correctness
- Refactoring is risky
- No regression testing

**Solution:** Implement unit tests and integration tests.

### 3.2 Design Issues

#### 1. **Dual Implementation Burden**
**Problem:** Maintaining both interpreter and compiler means:
- Every feature must be implemented twice
- Twice the bugs to fix
- Inconsistent behavior between modes

**Recommendation:** Choose one primary execution mode:
- **Interpreter:** Simpler, better for debugging, slower
- **VM:** Faster, more complex, industry-standard

#### 2. **Limited Error Messages**
**Problem:** Errors lack context:
```
Error : Import compilation not implemented.
```

**Better:**
```
Error at line 1: Import statements not supported in VM mode
  1 | import "mod.iz";
    | ^^^^^^^^^^^^^^^
Suggestion: Run with interpreter mode or remove import
```

**Solution:** Add source location tracking to all errors.

#### 3. **No Standard Library**
**Problem:** Only 2 native functions: `print()` and `len()`

**Impact:** Limited practical usefulness.

**Solution:** Add standard library modules:
- `std.io`: file I/O
- `std.string`: string manipulation
- `std.math`: mathematical functions
- `std.array`: array utilities

#### 4. **Memory Management Concerns**
**Problem:** Heavy use of `std::shared_ptr` for all runtime objects.

**Impact:** Reference counting overhead, potential cycles.

**Better Approach:** 
- Use garbage collection
- Or arena allocators for VM
- Document ownership clearly

### 3.3 Code Quality Issues

#### 1. **Inconsistent Naming**
```cpp
// Some use underscore
user_function.hpp

// Some use camelCase
ExprVisitor

// Some abbreviated
expr, stmt
```

**Recommendation:** Establish and document naming conventions.

#### 2. **Mixed Include Styles**
```cpp
#include "common/token.hpp"  // Relative
#include <memory>            // System
```

**Status:** Actually consistent, this is fine.

#### 3. **No Documentation**
**Problem:** 
- README is minimal (only "1.")
- No API documentation
- No language specification
- No examples beyond test.iz

**Solution:** Document:
- Language syntax and semantics
- API for embedding interpreter
- Build and development guide
- Contributing guidelines

---

## 4. Feature Roadmap

### Phase 1: Stabilization (Immediate)
**Goal:** Make the existing features work reliably

1. **Fix Import in VM Mode**
   - Implement `BytecodeCompiler::visit(ImportStmt&)`
   - Add module loading to VM
   - Test with mod.iz

2. **Add Test Suite**
   - Unit tests for lexer
   - Unit tests for parser
   - Integration tests for interpreter
   - Integration tests for VM

3. **Improve Error Messages**
   - Add source location to all errors
   - Show code snippet with caret
   - Add suggestions where applicable

4. **Document Existing Features**
   - Write language specification
   - Add code examples
   - Document standard library

### Phase 2: Language Completeness (Short-term)
**Goal:** Implement all planned language features

5. **Break/Continue**
   - Add AST nodes: `BreakStmt`, `ContinueStmt`
   - Update parser to recognize them
   - Implement in interpreter (use exceptions)
   - Implement in compiler (use jump instructions)

6. **Class System**
   - Design class semantics
   - Add AST nodes: `ClassStmt`, `ThisExpr`, `SuperExpr`
   - Implement in interpreter
   - Implement in compiler

7. **Standard Library**
   - Math functions: `sqrt`, `pow`, `sin`, `cos`, etc.
   - String functions: `split`, `join`, `substr`, etc.
   - Array functions: `push`, `pop`, `slice`, `map`, `filter`
   - File I/O: `open`, `read`, `write`, `close`

8. **Error Handling**
   - Add try/catch/throw syntax
   - Implement exception propagation
   - Standard exception types

### Phase 3: Advanced Features (Long-term)
**Goal:** Make IziLang production-ready

9. **Optimization**
   - Constant folding
   - Dead code elimination
   - Register allocation for VM
   - Inline caching for method calls

10. **Debugging Support**
    - Line number tracking in bytecode
    - Debugger protocol
    - Stack traces
    - Variable inspection

11. **Package Manager**
    - Package repository
    - Dependency resolution
    - Semantic versioning

12. **IDE Support**
    - Language server protocol (LSP)
    - Syntax highlighting
    - Auto-completion
    - Error checking

### Phase 4: Ecosystem (Future)
**Goal:** Build a community and ecosystem

13. **Web Playground**
    - WebAssembly compilation
    - Browser-based REPL
    - Share snippets

14. **Documentation Site**
    - Tutorial
    - API reference
    - Cookbook
    - Blog

15. **Community**
    - Contributing guide
    - Code of conduct
    - Discord/Slack
    - Regular releases

---

## 5. Recommendations

### 5.1 Immediate Actions

#### 1. Choose Primary Execution Mode
**Decision needed:** Interpreter OR VM?

**Recommendation:** **Focus on VM** because:
- ✅ Better performance
- ✅ Industry-standard approach
- ✅ More impressive technically
- ✅ Better for learning about compilers

**Action:** 
- Make VM fully functional first
- Keep interpreter for testing/comparison
- Eventually deprecate interpreter

#### 2. Fix Critical Bugs
Priority order:
1. Import in VM mode
2. Test suite setup
3. Error message improvement

#### 3. Improve Documentation
Minimum viable documentation:
- README with build instructions
- Language syntax reference
- 5-10 example programs

### 5.2 Code Architecture Improvements

#### 1. Separate Compilation Phases
```
Source → [Lexer] → Tokens
       → [Parser] → AST
       → [Semantic Analyzer] → Checked AST
       → [Code Generator] → Bytecode
       → [VM] → Execution
```

**Add missing phase:** Semantic Analyzer
- Type checking
- Variable resolution
- Constant evaluation

#### 2. Use Modern C++ Features More
```cpp
// Current: Manual variant checking
if (std::holds_alternative<double>(v)) {
    return std::get<double>(v);
}

// Better: std::visit
return std::visit([](auto&& arg) -> double {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, double>) {
        return arg;
    } else {
        throw std::runtime_error("Expected number");
    }
}, v);
```

#### 3. Add Formatter/Linter
- clang-format for consistent code style
- clang-tidy for static analysis
- Add to CI/CD

### 5.3 Testing Strategy

#### Unit Tests
```cpp
// Example: Lexer tests
TEST(Lexer, TokenizesNumbers) {
    Lexer lex("123.45");
    auto tokens = lex.scanTokens();
    ASSERT_EQ(tokens.size(), 2); // number + EOF
    EXPECT_EQ(tokens[0].type, TokenType::NUMBER);
    EXPECT_EQ(tokens[0].lexeme, "123.45");
}
```

**Framework:** Use Google Test or Catch2

#### Integration Tests
```cpp
TEST(Interpreter, FunctionCalls) {
    std::string code = R"(
        fn add(a, b) {
            return a + b;
        }
        print(add(2, 3));
    )";
    
    auto output = runCode(code);
    EXPECT_EQ(output, "5\n");
}
```

#### Fuzzing
- Random code generation
- Grammar-based fuzzing
- Crash detection

### 5.4 CI/CD Pipeline

**Recommended Setup:**
```yaml
# .github/workflows/ci.yml
name: CI

on: [push, pull_request]

jobs:
  build:
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: |
          # Install premake5
      - name: Build
        run: |
          premake5 gmake2
          make config=release
      - name: Test
        run: |
          ./bin/Release/izi/izi tests/
      - name: Upload artifacts
        uses: actions/upload-artifact@v2
```

---

## 6. Next Steps

### Immediate (This Week)
1. ✅ Complete this analysis
2. Fix import compilation in VM mode
3. Add basic test infrastructure
4. Update README with build instructions

### Short-term (This Month)
5. Implement break/continue
6. Add 10+ native functions
7. Write language specification
8. Set up CI/CD

### Medium-term (This Quarter)
9. Implement class system
10. Build standard library
11. Add error handling (try/catch)
12. Performance optimization

### Long-term (This Year)
13. LSP implementation
14. Package manager
15. Web playground
16. Community building

---

## 7. Conclusion

IziLang is a **promising programming language project** with:

### Strengths
- ✅ Clean, modern C++ codebase
- ✅ Good architectural patterns (Visitor, variant)
- ✅ Dual execution modes (learning opportunity)
- ✅ Core features working (variables, functions, control flow)

### Weaknesses
- ❌ Incomplete implementation (imports broken in VM)
- ❌ No test suite
- ❌ Limited documentation
- ❌ Missing standard library

### Potential
The project has excellent potential if:
1. Critical bugs are fixed
2. Feature set is completed
3. Testing and documentation improve
4. Community is built

### Priority Focus Areas
1. **Fix VM mode** (make it usable)
2. **Add tests** (prevent regressions)
3. **Document** (help users and contributors)
4. **Complete features** (break/continue, classes)
5. **Build ecosystem** (stdlib, tools, community)

---

**Assessment:** ⭐⭐⭐☆☆ (3/5)
- Current state: Early/Mid development
- Trajectory: Positive if maintained
- Recommendation: **Focus on stability before new features**

---

## Appendix A: Files Overview

### Core Files (Must Review)
- `src/main.cpp` - Entry point, execution mode selection
- `src/parse/lexer.cpp` - Token scanning
- `src/parse/parser.cpp` - AST construction
- `src/interp/interpreter.cpp` - Tree-walking interpreter
- `src/compile/compiler.cpp` - Bytecode compiler
- `src/bytecode/vm.cpp` - Virtual machine

### Supporting Files
- `src/ast/expr.hpp` - Expression node definitions
- `src/ast/stmt.hpp` - Statement node definitions
- `src/common/value.hpp` - Runtime value representation
- `src/common/token.hpp` - Token types
- `premake5.lua` - Build configuration

### Test Files
- `test.iz` - Main test script
- `mod.iz` - Import test module

---

## Appendix B: Useful Commands

```bash
# Build
premake5 gmake2
make config=debug

# Run
./bin/Debug/izi/izi test.iz

# Clean
make clean

# Count lines of code
find src -name "*.cpp" -o -name "*.hpp" | xargs wc -l

# Find todos
grep -rn "TODO\|FIXME" src/
```

---

**End of Analysis**

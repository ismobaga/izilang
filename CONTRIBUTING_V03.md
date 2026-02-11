# Contributing to v0.3 - Quick Start Guide

## 🎯 Goal

Help implement the remaining 65% of the v0.3 milestone. This guide shows you how to contribute effectively.

---

## 📋 Prerequisites

1. **Read the Foundation**
   - [V03_SUMMARY.md](V03_SUMMARY.md) - What's done
   - [V03_SPECIFICATION.md](docs/V03_SPECIFICATION.md) - Language spec
   - [V03_IMPLEMENTATION_STATUS.md](V03_IMPLEMENTATION_STATUS.md) - Detailed tasks

2. **Setup Development Environment**
   ```bash
   git clone https://github.com/ismobaga/izilang
   cd izilang
   ./premake5 gmake2
   make config=debug
   ./bin/Debug/tests/tests  # Verify all tests pass
   ```

3. **Understand the Architecture**
   - AST: `src/ast/` (expr.hpp, stmt.hpp, visitor.hpp, type.hpp)
   - Parser: `src/parse/` (lexer.cpp, parser.cpp)
   - Interpreter: `src/interp/` (interpreter.cpp)
   - Compiler: `src/compile/` (compiler.cpp)

---

## 🎯 Pick a Task

### 🟢 Good First Tasks (1-2 days each)

#### Task 1: Parse Type Annotations on Variables
**Goal**: Make `var x: Number = 10` work

**Files to modify**:
- `src/parse/parser.cpp` - Update `varDeclaration()` method

**Steps**:
1. After parsing variable name, check for `:`
2. If found, call new method `parseTypeAnnotation()`
3. Pass TypePtr to VarStmt constructor
4. Test with `var x: Number = 10;`

**Example**:
```cpp
// In Parser::varDeclaration()
TypePtr typeAnnotation = nullptr;
if (match(TokenType::COLON)) {
    typeAnnotation = parseTypeAnnotation();
}
return std::make_unique<VarStmt>(name, std::move(initializer), std::move(typeAnnotation));
```

**Test**:
```bash
echo 'var x: Number = 10; print(x);' > test_type.iz
./bin/Debug/izi/izi test_type.iz
```

#### Task 2: Parse Simple Class Declarations
**Goal**: Make `class Point { }` work

**Files to modify**:
- `src/parse/parser.cpp` - Add `classDeclaration()` method

**Steps**:
1. Add case for `CLASS` token in `declaration()`
2. Implement `classDeclaration()` method
3. Parse class name, `{`, fields, methods, `}`
4. Return `ClassStmt`

**Example**:
```cpp
StmtPtr Parser::classDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expected class name");
    consume(TokenType::LEFT_BRACE, "Expected '{' before class body");
    
    std::vector<std::unique_ptr<VarStmt>> fields;
    std::vector<std::unique_ptr<FunctionStmt>> methods;
    
    while (!check(TokenType::RIGHT_BRACE) && !isAtEnd()) {
        // Parse fields and methods
    }
    
    consume(TokenType::RIGHT_BRACE, "Expected '}' after class body");
    return std::make_unique<ClassStmt>(name.lexeme, std::move(fields), std::move(methods));
}
```

#### Task 3: Implement Property Access in Interpreter
**Goal**: Make `obj.field` work

**Files to modify**:
- `src/interp/interpreter.cpp` - Implement `visit(PropertyExpr&)`

**Steps**:
1. Evaluate object expression
2. Check if result is a class instance (Map for now)
3. Look up property in the instance
4. Return property value

---

### 🟡 Intermediate Tasks (3-5 days each)

#### Task 4: Implement Class Instantiation
**Goal**: Make `var p = Point(3, 4)` work

**What you need**:
- Understand how functions are stored in environment
- Create class instances as Maps (temporary)
- Bind methods to instance

**Files to modify**:
- `src/interp/interpreter.cpp` - `visit(ClassStmt&)`
- Create class constructor callable

#### Task 5: Implement `this` Binding
**Goal**: Make `this.x` work inside methods

**What you need**:
- Add `this` to environment when calling methods
- Handle `ThisExpr` visitor
- Bind instance to method context

**Files to modify**:
- `src/interp/interpreter.cpp` - `visit(ThisExpr&)`, method calls

#### Task 6: Add `izi check` Command
**Goal**: Make `izi check program.iz` run semantic analysis

**What you need**:
- Understand CLI system (`src/common/cli.cpp`)
- Integrate SemanticAnalyzer
- Format and print diagnostics

**Files to modify**:
- `src/common/cli.cpp` - Add check command
- `src/main.cpp` - Handle check mode

---

### 🔴 Advanced Tasks (1-2 weeks each)

#### Task 7: Bytecode VM Support for Classes
**Goal**: Make classes work in VM mode

**What you need**:
- Deep understanding of bytecode VM
- Design new opcodes (LOAD_PROPERTY, STORE_PROPERTY, LOAD_THIS)
- Update compiler and VM

**Files to modify**:
- `src/bytecode/opcode.hpp` - Add opcodes
- `src/compile/compiler.cpp` - Compile classes to bytecode
- `src/bytecode/vm.cpp` - Execute class opcodes

#### Task 8: Implement Mark-and-Sweep GC
**Goal**: Replace reference counting with proper GC

**What you need**:
- Understanding of GC algorithms
- Root tracking
- Mark phase
- Sweep phase

**Files to modify**:
- Create `src/gc/` directory
- `src/common/value.hpp` - Add GC integration
- All allocations route through GC

---

## 🔧 Development Workflow

### 1. Create a Branch
```bash
git checkout -b feature/parse-type-annotations
```

### 2. Write Tests First (TDD)
```bash
# Create test file
cat > tests/test_type_annotations.cpp << 'EOF'
#include "catch.hpp"
#include "parse/parser.hpp"

TEST_CASE("Parse variable with type annotation", "[parser][types]") {
    // Test here
}
EOF
```

### 3. Implement Feature
Edit the relevant files based on the task.

### 4. Build and Test
```bash
make config=debug
./bin/Debug/tests/tests [types]  # Run specific tests
./bin/Debug/tests/tests           # Run all tests
```

### 5. Manual Testing
```bash
# Create test program
echo 'var x: Number = 10; print(x);' > test.iz
./bin/Debug/izi/izi test.iz
```

### 6. Commit and Push
```bash
git add .
git commit -m "Add type annotation parsing for variables"
git push origin feature/parse-type-annotations
```

### 7. Create Pull Request
- Include tests
- Update documentation if needed
- Reference the v0.3 milestone issue

---

## 📝 Code Style

### Follow Existing Patterns

**Good**:
```cpp
// Use std::unique_ptr
std::unique_ptr<VarStmt> stmt = std::make_unique<VarStmt>(...);

// Use std::move
return std::make_unique<ClassStmt>(name, std::move(fields), std::move(methods));

// Visitor pattern
Value visit(SomeExpr& expr) override {
    // Implementation
}
```

**Avoid**:
```cpp
// Don't use raw pointers
VarStmt* stmt = new VarStmt(...);  // NO!

// Don't copy smart pointers
auto copy = stmt;  // NO! Use std::move
```

### Naming Conventions

- **Classes/Structs**: `PascalCase` (e.g., `TypeAnnotation`, `ClassStmt`)
- **Methods/Functions**: `camelCase` (e.g., `parseTypeAnnotation`, `visitClass`)
- **Variables**: `camelCase` (e.g., `typeAnnotation`, `className`)
- **Constants**: `UPPER_CASE` (e.g., `MAX_SIZE`)

---

## 🐛 Debugging Tips

### Print AST
```cpp
// Temporary debugging
std::cout << "Parsing type annotation for: " << name.lexeme << std::endl;
```

### Use GDB
```bash
gdb ./bin/Debug/izi/izi
(gdb) run test.iz
(gdb) break Parser::parseTypeAnnotation
(gdb) continue
(gdb) print typeAnnotation->kind
```

### Check Token Stream
```bash
# Add debug output in lexer
echo 'var x: Number = 10;' | ./bin/Debug/izi/izi --debug-tokens
```

---

## 📚 Helpful Resources

### Documentation
- [Custom Instructions](.github/agents/coding-instructions.md) - Coding conventions
- [AST Overview](src/ast/README.md) - AST structure (if exists)
- [Parser Guide](src/parse/README.md) - Parser patterns (if exists)

### Similar Code to Study
- **Type annotations**: Look at how function parameters are parsed
- **Classes**: Look at how function declarations work
- **Property access**: Look at how array indexing works (`expr[index]`)

### Ask for Help
- Open a GitHub Discussion
- Comment on the v0.3 milestone issue
- Tag maintainers in your PR

---

## ✅ Checklist Before Submitting PR

- [ ] All existing tests pass
- [ ] New tests written for your feature
- [ ] Code follows existing style
- [ ] No compiler warnings
- [ ] Documentation updated (if needed)
- [ ] CHANGELOG.md updated
- [ ] Tested manually with example programs

---

## 🏆 Recognition

Contributors to v0.3 will be:
- Listed in CHANGELOG.md
- Mentioned in release notes
- Added to CONTRIBUTORS.md

---

## 🎉 Example Contribution Flow

Here's what a full contribution looks like:

### 1. Pick Task
"I'll implement type annotation parsing for variables"

### 2. Read Docs
- Read V03_SPECIFICATION.md section on types
- Look at VarStmt in src/ast/stmt.hpp
- Understand how functions parse parameters

### 3. Write Test
```cpp
TEST_CASE("Parse variable with Number type", "[parser][types]") {
    std::string source = "var x: Number = 10;";
    Lexer lexer(source);
    Parser parser(lexer.scanTokens(), source);
    auto program = parser.parse();
    
    REQUIRE(program.size() == 1);
    auto* varStmt = dynamic_cast<VarStmt*>(program[0].get());
    REQUIRE(varStmt != nullptr);
    REQUIRE(varStmt->typeAnnotation != nullptr);
    REQUIRE(varStmt->typeAnnotation->kind == TypeAnnotation::Kind::Number);
}
```

### 4. Implement
Add `parseTypeAnnotation()` method to Parser class.

### 5. Test
```bash
make config=debug
./bin/Debug/tests/tests [types]
# All tests pass!
```

### 6. Manual Test
```bash
echo 'var x: Number = 10; print(x);' > test.iz
./bin/Debug/izi/izi test.iz
# Outputs: 10
```

### 7. Submit PR
Create PR with:
- Clear description
- Tests included
- Example usage

### 8. Celebrate! 🎉
You just contributed to an open-source programming language!

---

## 💬 Questions?

**Q: Where do I start?**  
A: Pick a "Good First Task" from this guide. Start with type annotation parsing.

**Q: How do I know if my implementation is correct?**  
A: Write tests! If tests pass and manual testing works, you're good.

**Q: What if I get stuck?**  
A: Check existing code for similar patterns. Ask in GitHub Discussions.

**Q: How long should a task take?**  
A: First contributions: 2-5 days. Experienced: 1-2 days.

**Q: Can I work on multiple tasks?**  
A: Finish one first, then pick another. Keeps PRs focused.

---

**Happy Coding!** 🚀

Let's build v0.3 together!

---

**Last Updated**: 2026-02-11  
**Status**: v0.3 Foundation Complete  
**Next Task**: Parser Updates

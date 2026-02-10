# IziLang - Next Steps Quick Reference

This document provides a quick action plan based on the comprehensive analysis in [ANALYSIS.md](ANALYSIS.md).

---

## 🚨 Critical Issues to Fix First

### 1. VM Import System (PRIORITY 1)
**Problem:** Default execution mode (VM) fails when code has import statements.

**Current Error:**
```
Error : Import compilation not implemented.
```

**Fix Location:** `src/compile/compiler.cpp:278`

**Action Items:**
- [ ] Implement `BytecodeCompiler::visit(ImportStmt&)`
- [ ] Design module loading strategy for VM
- [ ] Test with existing test files (test.iz imports mod.iz)

**Estimated Time:** 2-4 hours

---

### 2. Add Test Infrastructure (PRIORITY 2)
**Problem:** No automated testing. All testing is manual.

**Action Items:**
- [ ] Choose test framework (Google Test or Catch2)
- [ ] Set up test directory structure
- [ ] Add lexer unit tests
- [ ] Add parser unit tests
- [ ] Add interpreter integration tests
- [ ] Add VM integration tests
- [ ] Configure test runner in Makefile

**Estimated Time:** 1-2 days

---

### 3. Improve Error Messages (PRIORITY 3)
**Problem:** Errors lack context and source location.

**Current:**
```
Error : Expected number
```

**Better:**
```
Runtime Error at line 15, column 8:
  15 | var x = "hello" + 5;
     |         ^^^^^^^^^^^
Cannot add string and number
```

**Action Items:**
- [ ] Add source location tracking to all errors
- [ ] Implement error formatting with code snippets
- [ ] Add suggestion messages where applicable
- [ ] Color-code error output (optional)

**Estimated Time:** 4-6 hours

---

## 📝 Documentation Improvements

### 1. Update README.md
**Current:** Only contains "1."

**Needed Sections:**
```markdown
# IziLang

A modern programming language with clean syntax and dual execution modes.

## Features
- [List key features]

## Installation
- [Build instructions]

## Quick Start
- [Hello world example]
- [Basic syntax]

## Documentation
- See ANALYSIS.md for architecture details
- See examples/ for more examples

## Contributing
- [How to contribute]

## License
- [License info]
```

**Estimated Time:** 1-2 hours

---

### 2. Create Language Specification
**File:** `docs/LANGUAGE_SPEC.md`

**Sections Needed:**
- Syntax overview
- Data types
- Operators
- Control flow
- Functions
- Imports
- Standard library
- Examples

**Estimated Time:** 4-6 hours

---

## 🔧 Feature Implementation Priority

### Phase 1: Core Fixes (This Week)
1. ✅ Complete analysis (DONE)
2. ⬜ Fix VM imports
3. ⬜ Add test infrastructure
4. ⬜ Update documentation
5. ⬜ Improve error messages

### Phase 2: Missing Features (This Month)
6. ⬜ Implement break/continue
   - Add AST nodes
   - Update parser
   - Implement in interpreter
   - Implement in compiler
   
7. ⬜ Expand standard library
   - Math functions
   - String utilities
   - Array/Map helpers
   - File I/O basics

8. ⬜ Short-circuit evaluation
   - Fix AND operator
   - Fix OR operator
   - Add tests

### Phase 3: Advanced Features (This Quarter)
9. ⬜ Class system
   - Design class semantics
   - Add AST nodes
   - Implement inheritance
   - Implement methods

10. ⬜ Error handling (try/catch)
    - Add syntax
    - Implement exceptions
    - Standard exception types

11. ⬜ Optimization
    - Constant folding
    - Dead code elimination
    - Register allocation

### Phase 4: Tooling (Long-term)
12. ⬜ CI/CD setup
13. ⬜ Package manager
14. ⬜ LSP server for IDE support
15. ⬜ Web playground

---

## 🏗️ Recommended Development Workflow

### Daily Development
```bash
# 1. Build
make config=debug

# 2. Test
./bin/Debug/izi/izi test.iz
# (Once tests exist: make test)

# 3. Check for issues
grep -rn "TODO\|FIXME" src/

# 4. Commit frequently
git add .
git commit -m "Clear, descriptive message"
```

### Before Major Changes
1. Read relevant sections in ANALYSIS.md
2. Write tests first (TDD)
3. Implement minimal changes
4. Run all tests
5. Update documentation

### Before Pull Requests
1. Run all tests
2. Check code style
3. Update CHANGELOG
4. Update documentation
5. Squash commits if needed

---

## 📊 Progress Tracking

### Completion Status
- [ ] Phase 1: Core Fixes (0/5)
- [ ] Phase 2: Missing Features (0/3)
- [ ] Phase 3: Advanced Features (0/3)
- [ ] Phase 4: Tooling (0/4)

**Overall: 1/15 items complete (ANALYSIS.md)**

---

## 🎯 Success Criteria

### Short-term (1 month)
- ✅ VM mode works with imports
- ✅ 50+ unit tests passing
- ✅ Documentation complete
- ✅ CI/CD running

### Medium-term (3 months)
- ✅ All core features implemented
- ✅ Standard library with 20+ functions
- ✅ Break/continue working
- ✅ Error handling implemented

### Long-term (1 year)
- ✅ Class system complete
- ✅ Package manager functional
- ✅ LSP server for IDE support
- ✅ Active community (10+ contributors)

---

## 📞 Getting Help

### Resources
- **Full Analysis:** [ANALYSIS.md](ANALYSIS.md)
- **Code:** Browse `src/` directory
- **Examples:** `test.iz`, `mod.iz`

### Questions to Ask
- "What's the architecture of component X?"
  → See ANALYSIS.md Section 1
  
- "What features are missing?"
  → See ANALYSIS.md Section 2.2
  
- "What should I work on next?"
  → See Priority list above
  
- "How do I implement feature X?"
  → See ANALYSIS.md Section 5 (Recommendations)

---

## 🔄 Keeping This Updated

This file should be updated:
- ✅ When completing tasks (check boxes)
- ✅ When priorities change
- ✅ When adding new features to roadmap
- ✅ Weekly review of progress

**Last Updated:** 2026-02-10

---

**Quick Links:**
- 📄 [Full Analysis](ANALYSIS.md)
- 💻 [Source Code](src/)
- 🏗️ [Build Config](premake5.lua)
- 🧪 [Test Files](test.iz)

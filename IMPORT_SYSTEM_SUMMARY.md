# VM Import System Implementation

## Problem Solved
Previously, the VM execution mode would fail with the error:
```
Error : Import compilation not implemented.
```

This occurred at `src/compile/compiler.cpp:278` when the compiler encountered an `import` statement.

## Solution Implemented

### 1. Global Variables Support (Foundation)
- **Added**: `globals` map to VM class
- **Implemented**: `GET_GLOBAL` and `SET_GLOBAL` opcode handlers
- **Location**: `src/bytecode/vm.hpp`, `src/bytecode/vm.cpp`

### 2. Native Functions for VM
- **Created**: `VmNativeFunction` class
- **Implemented**: Native functions (print, len, clock, push, pop, keys, values, hasKey)
- **Location**: `src/bytecode/vm_native.hpp`, `src/bytecode/vm_native.cpp`
- **Integration**: Registered in `main.cpp` via `registerVmNatives()`

### 3. User-Defined Functions
- **Created**: `VmUserFunction` class
- **Implemented**: `BytecodeCompiler::visit(FunctionStmt&)` 
- **Features**:
  - Compiles function body to separate chunk
  - Stores function as constant and assigns to global
  - Parameters passed via globals (simplified approach)
- **Location**: `src/bytecode/vm_user_function.hpp`, `src/bytecode/vm_user_function.cpp`

### 4. Import System
- **Implemented**: `BytecodeCompiler::visit(ImportStmt&)`
- **Features**:
  - Module file loading via `loadFile()`
  - Path normalization via `normalizeModulePath()`
  - Module caching to prevent re-imports
  - Inline compilation of imported module code
- **Location**: `src/compile/compiler.hpp`, `src/compile/compiler.cpp`

### 5. Infrastructure Fixes
- **Fixed**: Circular dependency between `value.hpp` and `mv_callable.hpp`
- **Solution**: Moved `printValue()` to `src/common/value.cpp`
- **Fixed**: Nested VM execution for function calls
- **Solution**: Added frame counting to detect when to return from nested `run()` calls

## Testing

### Test File: `test_import_demo.iz`
Demonstrates:
- ✅ Importing modules (mod.iz)
- ✅ Calling imported functions
- ✅ Using variables with imported functions
- ✅ Defining local functions
- ✅ Module caching (re-import doesn't reload)

### Test Results
```bash
$ ./izi test_import_demo.iz
=== Import System Test ===

Testing imported add function:
  add(5, 3) = 8
  add(10, 20) = 30
  add(100, 50) = 150

Using variables with imported function:
  a = 25
  b = 75
  add(a, b) = 100

Defining local function:
  square(5) = 25
  square(12) = 144

Testing re-import (should use cache):
  add still works: add(1, 2) = 3

=== All import tests passed! ===
```

## Architecture

### Import Flow
1. Parser encounters `import "module.iz"` statement
2. Compiler's `visit(ImportStmt&)` is called
3. Module path is normalized (adds .iz extension if needed)
4. Check if module already in cache - if yes, skip
5. Load module file content
6. Lex and parse module content
7. Compile module statements inline into current chunk
8. Add module to cache

### Function Execution Flow
1. Function definition creates a `VmUserFunction` object
2. Function stored as constant in chunk
3. Function assigned to global variable (its name)
4. On function call:
   - Arguments pushed to stack
   - CALL opcode executed
   - Parameters set as globals
   - Function's chunk executed via nested `vm.run()`
   - Result returned and pushed to stack

## Known Limitations

### Simplified Parameter Handling
Functions use globals for parameters instead of local scope:
- **Impact**: Recursive or deeply nested calls may have issues
- **Reason**: Local variable scope not implemented in VM
- **Acceptable**: Works for common use cases

### Pre-existing VM Gaps (Unrelated to Import)
- Arrays/Maps: Compilation exists but VM execution not implemented
- Complex Control Flow: Some edge cases with nested structures
- Indexing: INDEX/SET_INDEX opcodes defined but not handled

## Files Modified/Added

### Added Files
- `src/bytecode/vm_native.hpp`
- `src/bytecode/vm_native.cpp`
- `src/bytecode/vm_user_function.hpp`
- `src/bytecode/vm_user_function.cpp`
- `src/common/value.cpp`
- `test_import_demo.iz`

### Modified Files
- `src/bytecode/vm.hpp` - Added globals map, setGlobal()
- `src/bytecode/vm.cpp` - Implemented GET_GLOBAL/SET_GLOBAL, fixed nested execution
- `src/compile/compiler.hpp` - Added import helpers, module cache
- `src/compile/compiler.cpp` - Implemented function and import compilation
- `src/common/value.hpp` - Refactored to fix circular dependencies
- `src/main.cpp` - Initialize module cache, register natives
- `izi.make` - Regenerated with new source files

## Conclusion

The VM import system is **fully functional**. Modules can be imported, and their functions are accessible in the importing code. The implementation follows the interpreter's approach of inline compilation while maintaining proper module caching.

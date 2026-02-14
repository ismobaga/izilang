# Runtime & VM Hardening

This document describes the runtime safety features and hardening measures implemented in IziLang v0.2.

## Overview

IziLang includes several runtime safety features to prevent common issues like stack overflows, excessive memory usage, and infinite recursion. These features are designed to provide better error messages and more robust execution.

## Runtime Safety Limits

### Stack Overflow Protection

**Feature**: Automatic detection and prevention of stack overflow from deep recursion.

**Details**:
- Maximum call depth: 256 frames (configurable via `MAX_CALL_DEPTH` constant)
- Applies to both the tree-walker interpreter and the bytecode VM
- Tracks recursion depth per function call

**Error Message**:
```
Error: Stack overflow: Maximum call depth of 256 exceeded.
```

**Example**:
```izilang
// This will trigger stack overflow protection
fn infiniteRecursion(n) {
    return infiniteRecursion(n + 1);
}

infiniteRecursion(0);  // Error after 256 calls
```

**Configuration**:
- Interpreter: `MAX_CALL_DEPTH` in `src/interp/interpreter.hpp`
- VM: `MAX_CALL_FRAMES` in `src/bytecode/vm.hpp`

### Stack Size Monitoring

**Feature**: Monitor the current stack depth and size during execution.

**API**:
- VM: `vm.getCallDepth()` and `vm.getStackSize()`
- Interpreter: `interp.callDepth` (public member)

**Usage**:
```cpp
// Check current call depth
size_t depth = vm.getCallDepth();
if (depth > threshold) {
    // Take action
}
```

## Memory Usage Metrics

### Memory Statistics Tracking

**Feature**: Track memory allocations and usage during program execution.

**Command-Line Flag**: `--memory-stats`

**Usage**:
```bash
izi run --memory-stats program.iz
```

**Output Example**:
```
╔════════════════════════════════════════╗
║     Memory Usage Report (Debug)       ║
╚════════════════════════════════════════╝

Overall Statistics:
  Total allocations:   1523
  Total deallocations: 1421
  Active allocations:  102
  Total bytes allocated: 45 KB
  Current memory usage:  3.2 KB
  Peak memory usage:     8.7 KB

Allocations by Type:
  Array: 342 allocations, 12 KB
  Map: 189 allocations, 8 KB
  String: 892 allocations, 18 KB
  Function: 100 allocations, 5 KB
```

**API**:
```cpp
#include "common/memory_metrics.hpp"

// Get memory metrics instance
auto& metrics = MemoryMetrics::getInstance();

// Record allocation
metrics.recordAllocation("Array", sizeof(Array));

// Record deallocation
metrics.recordDeallocation("Array", sizeof(Array));

// Print report
metrics.printReport();

// Reset metrics
metrics.reset();
```

### Debug Mode Features

When running with `--debug --memory-stats`, you get:
- Detailed allocation tracking
- Peak memory usage reporting
- Breakdown by object type
- Active allocation count

## Garbage Collection

### Current Status

IziLang uses reference-counted garbage collection via C++ smart pointers (`std::shared_ptr`):
- Automatic memory management
- Deterministic cleanup
- No explicit GC tuning needed for most programs

### Cyclic Structure Handling

**Note**: Reference cycles can cause memory leaks. Future versions will include:
- Cycle detection algorithms
- Weak references for breaking cycles
- Manual GC triggering API

**Current Workaround**:
- Avoid creating circular references
- Use manual cleanup when needed
- Keep object graphs acyclic

## Benchmarking Tools

### `izi bench` Command

**Purpose**: Measure execution time and performance of IziLang programs.

**Usage**:
```bash
# Basic benchmark
izi bench program.iz

# With VM engine
izi bench --vm program.iz

# Custom iterations
izi bench --iterations 10 program.iz

# With optimizations disabled
izi bench --no-optimize program.iz
```

**Output Example**:
```
╔════════════════════════════════════════╗
║   IziLang Performance Benchmark       ║
╚════════════════════════════════════════╝

File: benchmarks/arithmetic.iz
Engine: Interpreter
Iterations: 5
Optimizations: enabled

Running benchmark..... done!

Results:
  Total time:   1433 ms
  Average time: 286.6 ms
  Throughput:   3.49 runs/sec
```

**Best Practices**:
- Use release builds for accurate benchmarking: `make config=release`
- Run multiple iterations (default: 5) for stable results
- Compare with and without optimizations
- Use the VM mode (`--vm`) for production-like performance

### Benchmark Scripts

The `benchmarks/` directory includes:
- `arithmetic.iz`: Tests arithmetic and recursion
- `loops.iz`: Tests loop performance
- `functions.iz`: Tests function call overhead
- `arrays.iz`: Tests collection operations

**Running Benchmark Suite**:
```bash
./benchmarks/run_benchmarks.sh
```

## REPL Enhancements

### `:vars` Command

**Purpose**: Display all defined variables in the current REPL session.

**Usage**:
```
> var x = 10;
> var name = "Alice";
> :vars

Defined Variables:
  name = Alice
  x = 10
  (... built-in functions ...)
```

**Features**:
- Shows user-defined variables and built-in functions
- Lists all global variables and functions
- Displays current values
- Works in both interpreter and VM modes

### `:tasks` Command

**Purpose**: Display async tasks (placeholder for future async support).

**Usage**:
```
> :tasks

Async Tasks:
  (async/task support not yet implemented)
```

**Future Features**:
- List active async tasks
- Show task status (pending, running, complete)
- Display task priorities
- Cancel running tasks

### All REPL Commands

```
:help      Show help message
:exit      Exit the REPL
:reset     Reset the REPL environment
:debug     Toggle debug mode
:vars      Show all defined variables
:tasks     Show async tasks
```

## Performance Tips

### Optimization Flags

**Enable Optimizations** (default):
```bash
izi run program.iz
izi run --optimize program.iz
```

**Disable Optimizations**:
```bash
izi run --no-optimize program.iz
izi run -O0 program.iz
```

### VM vs. Interpreter

**Tree-Walker Interpreter** (default):
- Good for development
- Better error messages
- Easier debugging

**Bytecode VM**:
- 2-4x faster execution
- Better for production
- Use with `--vm` flag

**Example**:
```bash
izi run --vm --optimize program.iz
```

## Configuration Constants

### Adjustable Limits

Edit these constants in the source code if needed:

**Stack Limits**:
```cpp
// src/interp/interpreter.hpp
constexpr size_t MAX_CALL_DEPTH = 256;

// src/bytecode/vm.hpp
constexpr size_t MAX_CALL_FRAMES = 256;
constexpr size_t STACK_MAX = 256;
```

**After changing**:
```bash
make clean
make config=release
```

## Error Handling

### Stack Overflow Errors

**When it occurs**: Deep or infinite recursion

**Example**:
```izilang
fn factorial(n) {
    return n * factorial(n - 1);  // Missing base case!
}
factorial(1000);  // Stack overflow
```

**Fix**: Add proper base case
```izilang
fn factorial(n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
```

### Memory Issues

**Monitor with**:
```bash
izi run --debug --memory-stats program.iz
```

**Look for**:
- High peak memory usage
- Growing allocation count
- Memory not being released

## Future Enhancements

### Planned Features

1. **Garbage Collection Tuning**
   - Cycle detection for reference cycles
   - Configurable GC thresholds
   - GC pressure monitoring
   - Manual GC triggering

2. **Async/Task Support**
   - Async function declarations
   - Task queue management
   - Promise-like futures
   - Concurrent execution

3. **File Watching**
   - `izi run --watch` for auto-reload
   - Hot reloading during development
   - File change detection

4. **Advanced Memory Profiling**
   - Heap snapshots
   - Memory leak detection
   - Allocation flame graphs
   - Object lifetime tracking

## Troubleshooting

### Stack Overflow at Startup

**Symptom**: Error before any user code runs

**Cause**: Too many built-in functions or circular dependencies

**Fix**: Increase `MAX_CALL_DEPTH` or check for circular imports

### High Memory Usage

**Symptom**: Memory grows continuously

**Steps**:
1. Run with `--memory-stats`
2. Check for reference cycles
3. Look for large data structures
4. Profile with debug build

### Slow Benchmark Performance

**Symptom**: Benchmark takes too long

**Optimization**:
1. Use release build: `make config=release`
2. Enable optimizations: `--optimize`
3. Use VM mode: `--vm`
4. Reduce iterations if needed

## See Also

- [CLI Reference](CLI_REFERENCE.md) - Complete command-line options
- [Getting Started](GETTING_STARTED.md) - Basic usage guide
- [Performance Optimizations](../PERFORMANCE_OPTIMIZATIONS.md) - Optimization guide
- [Benchmarks README](../benchmarks/README.md) - Benchmarking details

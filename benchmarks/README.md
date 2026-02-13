# IziLang Performance Benchmarks

This directory contains benchmarks for measuring the performance of IziLang's optimization passes.

## Benchmark Suite

The following benchmarks are included:

### 1. Arithmetic Operations (`arithmetic.iz`)
Tests constant folding optimization for arithmetic expressions:
- Constant expression evaluation
- Recursive function calls (Fibonacci)
- Arithmetic in loops

### 2. Loop Performance (`loops.iz`)
Tests dead code elimination and loop optimization:
- Simple loops
- Nested loops
- Dead code in conditional branches

### 3. Function Calls (`functions.iz`)
Tests function call overhead and optimization:
- Simple function calls
- Nested function calls
- Recursive functions (factorial)

### 4. Array Operations (`arrays.iz`)
Tests collection performance:
- Array creation and population
- Element access
- Element modification

## Running Benchmarks

### Prerequisites
Build the release version of IziLang:
```bash
make config=release
```

### Run All Benchmarks
```bash
./benchmarks/run_benchmarks.sh
```

The script will:
1. Run each benchmark multiple times (default: 5 iterations)
2. Measure execution time with and without optimizations
3. Calculate average times and speedup ratios
4. Display results in a formatted table

### Run Individual Benchmarks

With optimization (default):
```bash
./bin/Release/izi/izi run benchmarks/arithmetic.iz
```

Without optimization:
```bash
./bin/Release/izi/izi run --no-optimize benchmarks/arithmetic.iz
```

With debug output to see optimization passes:
```bash
./bin/Release/izi/izi run --debug benchmarks/arithmetic.iz
```

## Optimization Passes

IziLang currently implements the following optimizations:

### 1. Constant Folding
Evaluates constant expressions at compile time:
- Binary operations: `2 + 3` → `5`
- Unary operations: `-5`, `!false`
- Comparison operations: `5 > 3` → `true`

### 2. Dead Code Elimination
Removes unreachable code:
- Statements after `return` in functions
- Unreachable branches in `if` statements with constant conditions
- `while` loops with constant `false` conditions

## Performance Expectations

With optimizations enabled, you should see:
- **Arithmetic benchmarks**: 5-15% improvement (constant folding)
- **Loop benchmarks**: 10-20% improvement (dead code elimination)
- **Function benchmarks**: 5-10% improvement (various optimizations)
- **Array benchmarks**: Minimal improvement (limited optimization opportunities)

## Adding New Benchmarks

To add a new benchmark:

1. Create a new `.iz` file in the `benchmarks/` directory
2. Write code that exercises specific language features
3. Add comments describing what is being tested
4. Run `./benchmarks/run_benchmarks.sh` to include it automatically

## Future Optimizations

Planned optimizations for future releases:
- **Inline Caching**: Speed up property lookups
- **Function Inlining**: Eliminate function call overhead for small functions
- **Loop Unrolling**: Optimize tight loops
- **JIT Compilation**: Compile hot paths to native code (stretch goal)

## Comparing with Other Languages

To compare IziLang performance with other languages, you can:

1. Port benchmarks to Python, Ruby, or JavaScript
2. Run them with their respective interpreters
3. Compare execution times

Example Python equivalent of `arithmetic.iz`:
```python
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n - 1) + fibonacci(n - 2)

# ... rest of the benchmark
```

## Tracking Performance Over Time

To track performance improvements:

1. Save benchmark results to a file:
   ```bash
   ./benchmarks/run_benchmarks.sh | tee results/$(date +%Y-%m-%d).txt
   ```

2. Compare results over time to identify regressions or improvements

3. Consider using a CI/CD pipeline to run benchmarks automatically

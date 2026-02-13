#!/bin/bash
# Simple benchmark runner for IziLang
# Compares performance with and without optimizations

set -e

# Configuration
BENCHMARK_DIR="benchmarks"
IZI_BIN="./bin/Release/izi/izi"
ITERATIONS=3

# Check if the izi binary exists
if [ ! -f "$IZI_BIN" ]; then
    echo "Error: IziLang binary not found at $IZI_BIN"
    echo "Please build the release version first:"
    echo "  make config=release"
    exit 1
fi

# Check if benchmark directory exists
if [ ! -d "$BENCHMARK_DIR" ]; then
    echo "Error: Benchmark directory not found at $BENCHMARK_DIR"
    exit 1
fi

echo "╔════════════════════════════════════════╗"
echo "║   IziLang Performance Benchmarks      ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Function to run a single benchmark and measure time
run_benchmark() {
    local benchmark=$1
    local optimize=$2
    local flag=""
    
    if [ "$optimize" = "true" ]; then
        flag="--optimize"
    else
        flag="--no-optimize"
    fi
    
    # Run the benchmark and measure time with bash's time
    local start=$(date +%s%N)
    for i in $(seq 1 $ITERATIONS); do
        $IZI_BIN run $flag "$BENCHMARK_DIR/$benchmark" > /dev/null 2>&1
    done
    local end=$(date +%s%N)
    
    # Calculate time in milliseconds
    local elapsed=$(((end - start) / 1000000))
    local avg=$((elapsed / ITERATIONS))
    
    echo "$avg"
}

# Results table header
printf "%-20s %-15s %-15s %-15s\n" "Benchmark" "No Optimize" "Optimized" "Difference"
echo "────────────────────────────────────────────────────────────────"

# Run benchmarks
for benchmark_file in $BENCHMARK_DIR/*.iz; do
    benchmark=$(basename "$benchmark_file")
    benchmark_name=$(basename "$benchmark" .iz)
    
    # Run without optimization
    time_no_opt=$(run_benchmark "$benchmark" false)
    
    # Run with optimization
    time_opt=$(run_benchmark "$benchmark" true)
    
    # Calculate difference (positive means optimization helped)
    diff=$((time_no_opt - time_opt))
    
    # Format with color based on performance
    if [ $diff -gt 0 ]; then
        diff_str="-${diff}ms (faster)"
    elif [ $diff -lt 0 ]; then
        diff_str="+${diff#-}ms (slower)"
    else
        diff_str="±0ms (same)"
    fi
    
    printf "%-20s %-15s %-15s %-15s\n" "$benchmark_name" "${time_no_opt}ms" "${time_opt}ms" "$diff_str"
done

echo ""
echo "════════════════════════════════════════"
echo "Benchmarking complete!"
echo ""
echo "Notes:"
echo "  - Each benchmark was run $ITERATIONS times and averaged"
echo "  - Times are in milliseconds"
echo "  - Negative difference indicates optimization improved performance"

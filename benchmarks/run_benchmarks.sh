#!/bin/bash
# Benchmark runner script for IziLang
# Measures performance with and without optimizations

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BENCHMARK_DIR="benchmarks"
IZI_BIN="./bin/Release/izi/izi"
ITERATIONS=5

# Check if the izi binary exists
if [ ! -f "$IZI_BIN" ]; then
    echo -e "${RED}Error: IziLang binary not found at $IZI_BIN${NC}"
    echo "Please build the release version first:"
    echo "  make config=release"
    exit 1
fi

# Check if benchmark directory exists
if [ ! -d "$BENCHMARK_DIR" ]; then
    echo -e "${RED}Error: Benchmark directory not found at $BENCHMARK_DIR${NC}"
    exit 1
fi

echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   IziLang Performance Benchmarks      ║${NC}"
echo -e "${BLUE}╔════════════════════════════════════════╗${NC}"
echo ""

# Function to run a single benchmark
run_benchmark() {
    local benchmark=$1
    local optimize=$2
    local flag=""
    
    if [ "$optimize" = "true" ]; then
        flag="--optimize"
    else
        flag="--no-optimize"
    fi
    
    # Run the benchmark multiple times and collect timing data
    local total_time=0
    
    for i in $(seq 1 $ITERATIONS); do
        # Use time command to measure execution time
        local exec_time=$(TIMEFORMAT='%3R'; { time $IZI_BIN run $flag "$BENCHMARK_DIR/$benchmark" > /dev/null 2>&1; } 2>&1)
        total_time=$(echo "$total_time + $exec_time" | bc)
    done
    
    # Calculate average time
    local avg_time=$(echo "scale=3; $total_time / $ITERATIONS" | bc)
    echo "$avg_time"
}

# Results table header
printf "${YELLOW}%-20s %-15s %-15s %-15s${NC}\n" "Benchmark" "No Optimize" "Optimized" "Speedup"
echo "────────────────────────────────────────────────────────────────"

# Run benchmarks
for benchmark in $(ls $BENCHMARK_DIR/*.iz | xargs -n 1 basename); do
    benchmark_name=$(basename "$benchmark" .iz)
    
    echo -n "Running $benchmark_name... "
    
    # Run without optimization
    time_no_opt=$(run_benchmark "$benchmark" false)
    
    # Run with optimization
    time_opt=$(run_benchmark "$benchmark" true)
    
    # Calculate speedup
    speedup=$(echo "scale=2; $time_no_opt / $time_opt" | bc)
    
    # Determine color based on speedup
    if (( $(echo "$speedup > 1.1" | bc -l) )); then
        color=$GREEN
    elif (( $(echo "$speedup < 0.9" | bc -l) )); then
        color=$RED
    else
        color=$NC
    fi
    
    printf "${color}%-20s %-15s %-15s %-15s${NC}\n" "$benchmark_name" "${time_no_opt}s" "${time_opt}s" "${speedup}x"
done

echo ""
echo -e "${BLUE}════════════════════════════════════════${NC}"
echo -e "${GREEN}Benchmarking complete!${NC}"
echo ""
echo "Notes:"
echo "  - Each benchmark was run $ITERATIONS times and averaged"
echo "  - Times are in seconds"
echo "  - Speedup > 1.0 indicates optimization improved performance"

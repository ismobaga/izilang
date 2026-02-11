# Testing Guide for IziLang

This document provides comprehensive information about testing the IziLang interpreter.

## Overview

IziLang uses the [Catch2](https://github.com/catchorg/Catch2) testing framework (v2.13.10) for both unit and integration tests. The test suite ensures reliability and correctness of the interpreter's core functionality.

## Building Tests

The test project is configured in `premake5.lua` and can be built alongside the main interpreter:

```bash
# Generate build files
./premake5 gmake2

# Build all projects (including tests) in debug mode
make config=debug

# Build in release mode
make config=release
```

The test executable will be generated at:
- Debug: `./bin/Debug/tests/tests`
- Release: `./bin/Release/tests/tests`

## Running Tests

### Basic Usage

```bash
# Run all tests
./bin/Debug/tests/tests

# Run with success messages for all assertions
./bin/Debug/tests/tests -s

# Run with extra verbose output
./bin/Debug/tests/tests -v high
```

### Filtering Tests

```bash
# Run only lexer tests
./bin/Debug/tests/tests [lexer]

# Run only value tests
./bin/Debug/tests/tests [value]

# Run only integration tests
./bin/Debug/tests/tests [integration]

# Run a specific test case
./bin/Debug/tests/tests "Lexer tokenizes numbers"

# Run multiple tags
./bin/Debug/tests/tests [lexer],[value]
```

### Listing Tests

```bash
# List all test cases
./bin/Debug/tests/tests --list-tests

# List all tags
./bin/Debug/tests/tests --list-tags

# List test names only
./bin/Debug/tests/tests --list-test-names-only
```

## Test Structure

### Unit Tests

#### Lexer Tests (`tests/test_lexer.cpp`)

Tests for the tokenization phase:
- Single character tokens (operators, brackets, etc.)
- Two character tokens (comparison operators, arrow)
- Number literals (integers, floating-point)
- String literals (including empty strings)
- Identifiers and keywords
- Line and column tracking
- Whitespace handling

#### Value Tests (`tests/test_value.cpp`)

Tests for the runtime value system:
- Type checking (nil, boolean, number, string, array, map)
- Truthiness evaluation
- Type name extraction
- Number conversion
- Array operations
- Map operations

### Integration Tests (`tests/test_integration.cpp`)

End-to-end tests that exercise the full interpreter pipeline:
- Arithmetic expressions
- Variable declaration and assignment
- String concatenation
- Boolean comparisons
- Control flow (if/else statements)
- Loops (while)
- Function declarations and calls
- Recursive functions
- Array creation and manipulation
- Map creation and manipulation

## Writing New Tests

### Test File Structure

```cpp
#include "catch.hpp"
#include "relevant/headers.hpp"

using namespace izi;

TEST_CASE("Description of what is being tested", "[tag]") {
    SECTION("Specific scenario") {
        // Arrange
        // Act
        // Assert
        REQUIRE(condition);
    }
}
```

### Common Assertions

```cpp
REQUIRE(expr);              // Test passes if expr is true
REQUIRE_FALSE(expr);        // Test passes if expr is false
REQUIRE_THROWS(expr);       // Test passes if expr throws any exception
REQUIRE_NOTHROW(expr);      // Test passes if expr doesn't throw
CHECK(expr);                // Like REQUIRE but continues on failure
```

### Test Tags

Use tags to categorize tests:
- `[lexer]` - Lexer/tokenization tests
- `[value]` - Value type tests
- `[integration]` - End-to-end integration tests
- Add custom tags as needed

## Test Coverage

Current test statistics:
- **21 test cases**
- **119 assertions**
- **100% pass rate**

Coverage by component:
- Lexer: 8 test cases
- Values: 6 test cases  
- Integration: 7 test cases

## Continuous Integration

Tests should be run:
- Before committing changes
- In CI/CD pipeline
- Before merging pull requests

## Troubleshooting

### Build Failures

If tests fail to compile:
1. Ensure all source files are up to date
2. Clean and rebuild: `make clean && make config=debug`
3. Verify Catch2 header is present in `tests/catch.hpp`

### Test Failures

If tests fail:
1. Run with verbose output: `./bin/Debug/tests/tests -s`
2. Run only the failing test to isolate the issue
3. Check for changes in lexer/parser/interpreter behavior
4. Update tests if behavior change is intentional

## Future Test Additions

Potential areas for additional test coverage:
- Parser tests (AST generation verification)
- Compiler tests (bytecode generation)
- VM tests (bytecode execution)
- Error handling and reporting
- Import/export system
- Class system (when implemented)
- More edge cases and boundary conditions

## Resources

- [Catch2 Documentation](https://github.com/catchorg/Catch2/blob/v2.x/docs/Readme.md)
- [Catch2 Tutorial](https://github.com/catchorg/Catch2/blob/v2.x/docs/tutorial.md)

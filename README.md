

# IziLang

A programming language interpreter with improved error reporting.

## Features

### Improved Error Messages

IziLang provides clear, context-rich error messages with:
- **Source location tracking**: Errors show line and column numbers
- **Code snippets**: See the exact line where the error occurred  
- **Visual indicators**: Carets (^) point to the error location
- **Color-coded output**: Errors are highlighted for easy reading
- **Descriptive messages**: Clear explanations of what went wrong

Example error output:
```
Runtime Error at line 3, column 18:
  3 | var result = name + age;
    |                  ^
Cannot add string and number. Operands must be two numbers or two strings.
```

### Build Instructions

```bash
# Generate build files
./premake5 gmake2

# Build debug version
make config=debug

# Build release version  
make config=release

# Run
./bin/Debug/izi/izi test.iz
```

### Testing

The project includes comprehensive unit and integration tests using the Catch2 framework.

#### Running Tests

```bash
# Build the tests
make config=debug

# Run all tests
./bin/Debug/tests/tests

# Run specific tests by tag
./bin/Debug/tests/tests [lexer]
./bin/Debug/tests/tests [value]
./bin/Debug/tests/tests [integration]

# List all available tests
./bin/Debug/tests/tests --list-tests

# Run tests with verbose output
./bin/Debug/tests/tests -s
```

#### Test Coverage

The test suite includes:

- **Unit Tests**:
  - Lexer tests: Tokenization of operators, keywords, literals, strings, and numbers
  - Value tests: Type checking, truthiness, and operations on different value types
  
- **Integration Tests**:
  - Arithmetic expressions (addition, subtraction, multiplication, division)
  - Variable declaration and assignment
  - String operations and concatenation
  - Boolean operations and comparisons
  - Control flow (if/else statements)
  - Loops (while loops)
  - Functions (declaration, calls, recursion)
  - Arrays (creation, access, assignment)
  - Maps (creation, access, assignment)
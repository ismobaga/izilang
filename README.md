

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
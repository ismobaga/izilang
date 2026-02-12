# Native Compilation Example

This example demonstrates the native compilation feature of IziLang.

## What is Native Compilation?

Native compilation allows you to compile IziLang source files into standalone executables that:
- Have no runtime dependencies
- Are fully statically linked
- Can be distributed without requiring IziLang to be installed
- Run natively on the target platform

## Usage

```bash
# Compile the example
izi compile compile_example.iz -o demo

# Run the compiled executable
./demo
```

## Example Output

```
=== IziLang Compiled Executable Demo ===

Hello, World!

Factorial calculations:
  1! = 1
  2! = 2
  3! = 6
  ...
  10! = 3628800

Array operations:
  Array: [1, 2, 3, 4, 5]
  Sum: 15
  Length: 5

String operations:
  Full name: John Doe
  Length: 8

=== Demo Complete ===

This executable is:
  - Fully statically linked
  - Has no runtime dependencies
  - Can be distributed standalone
```

## Verification

You can verify the executable is statically linked:

```bash
# Check for dynamic dependencies (should show "not a dynamic executable")
ldd ./demo

# Check file type (should show "statically linked")
file ./demo
```

## Features Demonstrated

- Functions and recursion
- Variables and arithmetic
- Loops (while)
- Arrays and array operations
- String concatenation
- Built-in functions (print, str, len)

## Binary Size

Compiled executables are approximately 3-4 MB in size due to the embedded IziLang interpreter.

## Platform Support

Native compilation is currently supported on:
- Linux (tested with GCC and Clang)
- May work on macOS and Windows with appropriate compiler installed

## Technical Details

The native compiler:
1. Validates the IziLang source code
2. Generates C++ code that embeds the source
3. Compiles the C++ code with static linking
4. Produces a standalone executable

The executable includes:
- The IziLang interpreter
- All standard library functions
- Your embedded source code
- Statically linked C/C++ runtime libraries

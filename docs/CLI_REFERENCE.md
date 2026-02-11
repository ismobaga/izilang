# IziLang CLI Reference

Complete reference for all IziLang command-line interface commands and options.

## Synopsis

```
izi [command] [options] [arguments]
```

## Commands

### `run` - Execute a Source File

Execute an IziLang source file.

**Usage:**
```bash
izi run [options] <file>
# Shorthand (default command):
izi <file>
```

**Options:**
- `--vm` - Use bytecode VM for execution
- `--interp` - Use tree-walker interpreter (default)
- `--debug` - Enable debug/verbose output

**Examples:**
```bash
# Run with interpreter (default)
izi run script.iz
izi script.iz

# Run with VM
izi run --vm script.iz

# Run with debug output
izi run --debug script.iz
```

**Exit Codes:**
- `0` - Success
- `1` - Runtime error, file not found, or other error

---

### `build` - Compile/Check Without Executing

Parse and compile the source file without executing it. Useful for checking syntax and compilation errors before running.

**Usage:**
```bash
izi build [options] <file>
```

**Options:**
- `--debug` - Show compilation details

**Examples:**
```bash
# Check if code compiles
izi build app.iz

# Show compilation details
izi build --debug app.iz
```

**Exit Codes:**
- `0` - Build successful
- `1` - Syntax error, compilation error, or file not found

**Output:**
- On success: `Build successful: <filename>`
- On error: Detailed error message with file, line, and column

---

### `check` - Parse and Analyze

Perform quick syntax checking without compilation or execution. Fastest way to validate syntax.

**Usage:**
```bash
izi check [options] <file>
```

**Options:**
- `--debug` - Show parse tree details

**Examples:**
```bash
# Quick syntax check
izi check script.iz

# Show parsing details
izi check --debug script.iz
```

**Exit Codes:**
- `0` - Check successful
- `1` - Syntax error or file not found

**Output:**
- On success: `Check successful: <filename>`
- On error: Detailed error message with file, line, and column

---

### `test` - Run Test Files

Discover and execute test files from the `examples/` and `tests/` directories.

**Usage:**
```bash
izi test [options] [pattern]
```

**Arguments:**
- `pattern` - Optional pattern to filter test files (substring match)

**Options:**
- `--vm` - Use bytecode VM for tests
- `--debug` - Show detailed test output

**Examples:**
```bash
# Run all tests
izi test

# Run tests matching 'collections'
izi test collections

# Run tests with VM
izi test --vm

# Show test output
izi test --debug
```

**Exit Codes:**
- `0` - All tests passed
- `1` - One or more tests failed

**Output:**
```
Running 4 test file(s)...

Testing: examples/collections_demo.iz ... PASSED
Testing: examples/module_demo.iz ... PASSED
Testing: examples/pattern_matching_demo.iz ... PASSED
Testing: examples/string_interpolation.iz ... PASSED

========================================
Test Results
========================================
Passed: 4
Failed: 0
Total:  4
```

---

### `repl` - Interactive REPL

Start an interactive Read-Eval-Print Loop for experimenting with IziLang code.

**Usage:**
```bash
izi repl [options]
# Or simply:
izi
```

**Options:**
- `--vm` - Use bytecode VM
- `--debug` - Enable debug output

**Examples:**
```bash
# Start REPL
izi repl
izi

# Start REPL with VM
izi repl --vm
```

**REPL Commands:**
- `:help` - Show available REPL commands
- `:exit` - Exit the REPL
- `:reset` - Reset the REPL environment
- `:debug` - Toggle debug mode
- `exit()`, `quit()` - Exit the REPL
- `Ctrl+D` - Exit the REPL

**Features:**
- Multi-line input (lines ending with `{` or `(`)
- Error recovery (errors don't crash the REPL)
- Persistent environment per session
- Command history (future enhancement)

**Example Session:**
```
IziLang 0.2.0 REPL
Type 'exit()' or press Ctrl+D to quit
Type ':help' for REPL commands

> var x = 10;
> print(x * 2);
20
> fn add(a, b) {
... return a + b;
... }
> print(add(5, 3));
8
> :exit
```

---

### `fmt` - Format Source Code

*(Coming in v0.2 - not yet implemented)*

Format IziLang source code according to standard style.

**Usage:**
```bash
izi fmt [options] <file>
```

**Options:**
- `--check` - Check if file needs formatting (no changes)
- `--write` - Write changes to file (default: print to stdout)

---

### `version` - Show Version

Display IziLang version information.

**Usage:**
```bash
izi version
izi --version
izi -v
```

**Output:**
```
IziLang 0.2.0
```

---

### `help` - Show Help

Display help information for IziLang or a specific command.

**Usage:**
```bash
izi help [command]
izi --help
izi -h
```

**Examples:**
```bash
# Show general help
izi help
izi --help

# Show help for a specific command
izi help run
izi help build
izi help test
```

---

## Global Options

These options can be used with most commands:

- `--vm` - Use bytecode VM instead of tree-walker interpreter
- `--interp` - Use tree-walker interpreter (default)
- `--debug` - Enable debug/verbose output
- `--help`, `-h` - Show help message
- `--version`, `-v` - Show version information

---

## Environment Variables

Currently, IziLang doesn't use environment variables. This may change in future versions.

---

## Configuration Files

### `.izifmt.toml` *(Coming in v0.2)*

Configuration file for the code formatter.

Example:
```toml
indent_size = 4
line_length = 100
```

---

## Exit Codes

All IziLang commands use standard exit codes:

- `0` - Success
- `1` - Error (syntax error, runtime error, file not found, etc.)

---

## Error Messages

IziLang provides detailed error messages with:

- **File name** - Which file contains the error
- **Line and column** - Exact location of the error
- **Source context** - The line of code with the error
- **Visual indicator** - Caret (^) pointing to the error
- **Description** - Clear explanation of what went wrong

Example:
```
In file 'script.iz':
Runtime Error at line 3, column 10:
  3 | var z = x + y;
    |          ^
Cannot add number and string. Operands must be two numbers or two strings.
```

---

## Tips and Best Practices

### Quick Syntax Check

Use `izi check` for the fastest syntax validation:
```bash
izi check *.iz
```

### Development Workflow

1. **Write** - Edit your code
2. **Check** - `izi check file.iz` for quick validation
3. **Build** - `izi build file.iz` to check compilation
4. **Run** - `izi run file.iz` to execute

### Testing

Organize test files in `examples/` or `tests/` directory and run with:
```bash
izi test
```

### REPL for Experimentation

Use the REPL to quickly test ideas:
```bash
izi repl
> var x = [1, 2, 3];
> print(len(x));
3
```

---

## See Also

- [Getting Started Guide](GETTING_STARTED.md)
- [Editor Setup](EDITOR_SETUP.md)
- [Language Reference](LANGUAGE_REFERENCE.md)
- [Troubleshooting](ERRORS_TROUBLESHOOTING.md)

---

**IziLang** - A modern, expressive programming language

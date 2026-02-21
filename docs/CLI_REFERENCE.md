# IziLang CLI Reference

Complete reference for all IziLang command-line interface commands and options.

## Synopsis

```
izi [command] [options] [arguments]
```

## Quick Reference

| Command | Description |
|---------|-------------|
| `run` | Execute a source file (.iz or .izb) |
| `build` | Compile/check without executing |
| `check` | Parse and analyze (fastest validation) |
| `compile` | Compile to native executable |
| `chunk` | Compile to bytecode chunk (.izb) |
| `test` | Run test files |
| `repl` | Interactive Read-Eval-Print Loop |
| `fmt` | Format source code (coming soon) |
| `version` | Show version information |
| `help` | Show help for commands |

## Commands

### `run` - Execute a Source File

Execute an IziLang source file (`.iz`) or bytecode chunk file (`.izb`).

**Usage:**
```bash
izi run [options] <file>
# Shorthand (default command):
izi <file>
```

**Options:**
- `--vm` - Use bytecode VM for execution (required for .izb files)
- `--interp` - Use tree-walker interpreter (default for .iz files)
- `--debug` - Enable debug/verbose output

**Examples:**
```bash
# Run with interpreter (default for .iz)
izi run script.iz
izi script.iz

# Run with VM
izi run --vm script.iz

# Run bytecode file (requires --vm)
izi run --vm app.izb

# Run with debug output
izi run --debug script.iz
```

**File Types:**
- `.iz` - Source files (can run with interpreter or VM)
- `.izb` - Bytecode chunk files (VM only, faster loading)

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

### `compile` - Compile to Native Executable

Compile an IziLang source file into a standalone native executable with no runtime dependencies. The resulting executable is fully statically linked and can be distributed without requiring IziLang or any runtime libraries to be installed.

**Usage:**
```bash
izi compile [options] <file> [-o <output>]
```

**Arguments:**
- `<file>` - IziLang source file to compile
- `-o <output>` - Output executable name (optional, defaults to source filename without extension)

**Options:**
- `-o <name>` - Specify output executable name
- `--debug` - Include debug symbols in the executable

**Examples:**
```bash
# Compile to executable with default name
izi compile app.iz
# Creates executable: ./app

# Specify output name
izi compile app.iz -o myapp
# Creates executable: ./myapp

# Compile with debug symbols
izi compile --debug app.iz -o app_debug
```

**Exit Codes:**
- `0` - Compilation successful
- `1` - Compilation error, syntax error, or file not found

**Output:**
```
Validating source code...
Source code validated successfully.
Compiling to native executable...
Successfully compiled to: myapp
```

**Features:**
- **No runtime dependencies** - Fully statically linked executable
- **Standalone distribution** - Share executables without requiring IziLang installation
- **Cross-platform** - Compiles for the host platform (Linux, macOS, Windows)
- **Optimized binaries** - Release builds are optimized for performance

**Technical Details:**
- Uses C++20 compiler (g++, clang++, or MSVC)
- Static linking of all C/C++ runtime libraries
- Embeds IziLang source code directly into the executable
- Includes the IziLang interpreter for execution

**Requirements:**
- C++20 compatible compiler installed
- Source files for IziLang interpreter (automatically located)

**Limitations:**
- Executable size is larger (~3-4MB) due to embedded interpreter
- Compilation time depends on C++ compiler optimization level
- Source code is embedded (not compiled to machine code directly)

---

### `chunk` - Compile to Bytecode Chunk

Compile an IziLang source file into a bytecode chunk file (`.izb`) that can be executed efficiently by the VM without reparsing. This is similar to Python's `.pyc` files or Lua's compiled chunks.

**Usage:**
```bash
izi chunk [options] <file> [-o <output>]
```

**Arguments:**
- `<file>` - IziLang source file to compile
- `-o <output>` - Output .izb file name (optional, defaults to source filename with .izb extension)

**Options:**
- `-o <name>` - Specify output bytecode file name
- `--debug` - Show compilation details

**Examples:**
```bash
# Compile to bytecode with default name
izi chunk app.iz
# Creates: app.izb

# Specify output name
izi chunk app.iz -o application.izb

# Compile with debug information
izi chunk --debug script.iz
```

**Running Bytecode Files:**
```bash
# Execute .izb file (VM required)
izi run --vm app.izb

# .izb files can only run with --vm flag
izi run --vm script.izb
```

**Importing Bytecode Modules:**
```bash
# If mymodule.izb exists, it will be preferred over mymodule.iz
import "mymodule";
```

**Exit Codes:**
- `0` - Compilation successful
- `1` - Compilation error, syntax error, or file not found

**Output:**
```
[DEBUG] Compiling to bytecode chunk...
[DEBUG] Lexing complete, 52 tokens
[DEBUG] Parsing complete, 6 statements
[DEBUG] Applying optimizations...
[DEBUG] Bytecode compilation complete
[DEBUG] Code size: 47 bytes
[DEBUG] Constants: 6
[DEBUG] Names: 10
Successfully compiled to: app.izb
```

**Features:**
- **Faster Loading** - Skip parsing and lexing during execution
- **Module Precompilation** - Precompile frequently imported modules
- **Auto-Discovery** - VM automatically uses .izb files when available
- **Optimization Preserved** - Optimizations are baked into bytecode
- **VM Execution Only** - Requires `--vm` flag to execute

**Binary Format:**
- Magic number: `IZB\0` (4 bytes)
- Version: uint32 (format version)
- Code section: Bytecode instructions
- Constants section: Serialized values
- Names section: Global variable and function names

**Use Cases:**
- **Distribution** - Ship precompiled modules for faster startup
- **Library Development** - Precompile standard libraries
- **Production Deployments** - Reduce parsing overhead in production
- **Module Caching** - Cache compiled versions of large modules

**Limitations:**
- `.izb` files are platform-independent (bytecode is portable)
- Format version must match (incompatible versions will error)
- Native functions cannot be serialized (must be registered at runtime)
- Instance objects cannot be serialized (runtime-only constructs)

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
IziLang 0.3.0 REPL
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

*(Coming in v0.4 - not yet implemented)*

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

### `.izifmt.toml` *(Coming in v0.4)*

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
- [Language Specification](V03_SPECIFICATION.md)
- [Troubleshooting](ERRORS_TROUBLESHOOTING.md)

---

**IziLang** - A modern, expressive programming language

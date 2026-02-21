# Errors and Troubleshooting

This guide helps you understand and fix common errors in IziLang.

## Understanding Error Messages

IziLang provides detailed error messages with:

1. **File location** - Which file contains the error
2. **Line and column** - Exact position of the error
3. **Source context** - The problematic line of code
4. **Visual indicator** - Caret (^) pointing to the error location
5. **Description** - Clear explanation of the problem

Example error:
```
In file 'script.iz':
Runtime Error at line 3, column 10:
  3 | var z = x + y;
    |          ^
Cannot add number and string. Operands must be two numbers or two strings.
```

## Common Errors

### Syntax Errors

#### Unexpected Character

**Error:**
```
Lexer Error at line 2, column 0:
  2 | @ invalid token
    | ^
Unexpected character '@'
```

**Cause:** Invalid character in source code.

**Solution:** Remove or replace the invalid character. IziLang accepts letters, digits, and standard operators.

---

#### Unterminated String

**Error:**
```
Lexer Error at line 5, column 10:
  5 | var name = "Alice;
    |            ^
Unterminated string
```

**Cause:** String literal missing closing quote.

**Solution:** Add the closing quote:
```izilang
var name = "Alice";  // Fixed
```

---

#### Unterminated Comment

**Error:**
```
Lexer Error at line 10, column 0:
  10 | /* This comment never ends...
     | ^
Unterminated block comment
```

**Cause:** Block comment `/* */` not closed.

**Solution:** Add the closing `*/`:
```izilang
/* This comment is complete */
```

---

### Type Errors

#### Cannot Add Mismatched Types

**Error:**
```
Runtime Error at line 3, column 10:
  3 | var z = x + y;
    |          ^
Cannot add number and string. Operands must be two numbers or two strings.
```

**Cause:** Trying to add incompatible types (e.g., number + string).

**Solution:** Convert to compatible types:
```izilang
// Option 1: Convert number to string
var x = 10;
var y = "hello";
var z = toString(x) + y;  // "10hello"

// Option 2: Use only numbers
var x = 10;
var y = 20;
var z = x + y;  // 30
```

---

#### Expected Number

**Error:**
```
Runtime Error at line 5, column 5:
  5 | var result = -name;
    |              ^
Expected number, got string.
```

**Cause:** Operator requires a number but got another type.

**Solution:** Use the correct type:
```izilang
var value = 10;
var result = -value;  // Correct
```

---

### Variable Errors

#### Undefined Variable

**Error:**
```
Runtime Error at line 2, column 0:
  2 | print(unknownVar);
    |       ^
Undefined variable 'unknownVar'.
```

**Cause:** Using a variable before it's declared.

**Solution:** Declare the variable first:
```izilang
var unknownVar = 42;
print(unknownVar);  // Works
```

---

#### Cannot Reassign to Undefined

**Error:**
```
Runtime Error at line 2, column 0:
  2 | x = 10;
    | ^
Undefined variable 'x'.
```

**Cause:** Trying to assign to a variable that doesn't exist.

**Solution:** Declare the variable first:
```izilang
var x = 10;  // Declare first
x = 20;      // Now you can reassign
```

---

### Function Errors

#### Wrong Number of Arguments

**Error:**
```
Runtime Error at line 5, column 0:
  5 | result = add(5);
    |          ^
Expected 2 arguments but got 1.
```

**Cause:** Function called with incorrect number of arguments.

**Solution:** Provide the correct number of arguments:
```izilang
fn add(x, y) {
    return x + y;
}

var result = add(5, 3);  // Correct: 2 arguments
```

---

#### Not a Function

**Error:**
```
Runtime Error at line 3, column 0:
  3 | result = notAFunction();
    |          ^
Can only call functions and classes.
```

**Cause:** Trying to call something that isn't a function.

**Solution:** Make sure the value is a function:
```izilang
fn myFunction() {
    return 42;
}

var result = myFunction();  // Correct
```

---

### Array/Collection Errors

#### Index Out of Bounds

**Error:**
```
Runtime Error at line 3, column 0:
  3 | var item = arr[10];
    |            ^
Index out of bounds.
```

**Cause:** Accessing an array index that doesn't exist.

**Solution:** Check array bounds:
```izilang
var arr = [1, 2, 3];

// Check bounds before accessing
if (index < len(arr)) {
    var item = arr[index];
}
```

---

#### Cannot Index Non-Array

**Error:**
```
Runtime Error at line 2, column 0:
  2 | var item = x[0];
    |            ^
Can only index arrays and maps.
```

**Cause:** Trying to use index notation on non-indexable type.

**Solution:** Only index arrays and maps:
```izilang
var arr = [1, 2, 3];
var item = arr[0];  // Correct

var map = {key: "value"};
var val = map["key"];  // Correct
```

---

### Import Errors

#### Cannot Import Module

**Error:**
```
Runtime Error at line 1, column 0:
  1 | import "nonexistent.iz";
    |        ^
Cannot open file 'nonexistent.iz'.
```

**Cause:** Imported file doesn't exist or path is incorrect.

**Solution:** 
1. Check file path is correct
2. Verify file exists
3. Use relative paths for local files:
   ```izilang
   import "./utils.iz";
   import "../lib/math.iz";
   ```

---

## File Not Found Errors

### Cannot Open File

**Error:**
```
Cannot open file: script.iz
```

**Cause:** File doesn't exist or path is incorrect.

**Solution:**
1. Check filename spelling
2. Verify file exists: `ls script.iz`
3. Use absolute or correct relative path
4. Check file permissions: `ls -l script.iz`

---

## Build Errors

### Build Failed

**Error:**
```
Build failed: <error message>
```

**Cause:** Compilation error in the code.

**Solution:**
1. Run `izi check` for quick syntax validation
2. Fix reported syntax errors
3. Try `izi build` again

---

## REPL Errors

### Cannot Enter Multiline

**Issue:** Multiline input not working in REPL.

**Solution:** Lines ending with `{` or `(` automatically trigger multiline mode:
```
> fn hello() {
... return "Hello";
... }
```

---

### REPL State Confusion

**Issue:** Variables from previous commands causing unexpected behavior.

**Solution:** Use `:reset` to clear the REPL environment:
```
> :reset
REPL environment reset.
```

---

## Performance Issues

### Slow Execution

**Issue:** Code runs slowly.

**Solution:**
1. Try the VM mode: `izi run --vm script.iz`
2. Optimize loops and recursive functions
3. Reduce unnecessary computations
4. Profile code to find bottlenecks

---

### High Memory Usage

**Issue:** Program uses too much memory.

**Solution:**
1. Check for memory leaks in native functions
2. Limit array/map sizes
3. Clear unused references
4. Use streaming for large data

---

## Debugging Tips

### Enable Debug Mode

Use `--debug` flag to see detailed execution information:
```bash
izi run --debug script.iz
```

This shows:
- Lexing and parsing steps
- Execution mode (interpreter vs VM)
- Compilation details
- Additional diagnostic information

---

### Check Syntax First

Before running, validate syntax:
```bash
# Quick check
izi check script.iz

# Full compilation check
izi build script.iz

# Then run
izi run script.iz
```

---

### Use the REPL for Experimentation

Test small pieces of code in the REPL:
```bash
izi repl
> var x = 10;
> print(x * 2);
20
```

---

### Isolate the Problem

1. Comment out code sections
2. Test each part individually
3. Add print statements for debugging
4. Use binary search to find problematic code

---

## Platform-Specific Issues

### Linux/macOS

#### Permission Denied

**Error:**
```
bash: ./izi: Permission denied
```

**Solution:**
```bash
chmod +x izi
```

#### Library Not Found

**Error:**
```
error while loading shared libraries
```

**Solution:**
Install required libraries:
```bash
# Ubuntu/Debian
sudo apt-get install libstdc++6

# Fedora/RHEL
sudo dnf install libstdc++
```

---

### Windows

#### Missing DLL

**Error:**
```
The code execution cannot proceed because VCRUNTIME140.dll was not found.
```

**Solution:**
Install Visual C++ Redistributable:
- Download from Microsoft website
- Install for your platform (x64/x86)

---

## Getting More Help

If you can't resolve an issue:

1. **Check Examples** - Look at working code in `examples/`
2. **Read Documentation** - Review relevant docs
3. **Search Issues** - Check GitHub issues for similar problems
4. **Ask for Help** - Open a new GitHub issue with:
   - Error message (complete output)
   - Minimal code to reproduce
   - Platform and version info
   - What you've tried

### Minimal Reproducible Example

When reporting issues, provide:

```izilang
// What I'm trying to do:
// Calculate fibonacci numbers

fn fib(n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

print(fib(10));  // Error occurs here

// Error message:
// [paste complete error output]

// System info:
// IziLang version: 0.2.0
// OS: Linux 5.15
// Command: izi run fib.iz
```

---

## Common Pitfalls

### 1. Forgetting Variable Declaration

❌ Wrong:
```izilang
x = 10;  // Error: undefined variable
```

✅ Correct:
```izilang
var x = 10;  // Declare first
```

---

### 2. Mixing Types in Operations

❌ Wrong:
```izilang
var result = 10 + "20";  // Error: type mismatch
```

✅ Correct:
```izilang
var result = 10 + 20;  // Use same types
```

---

### 3. Array Bounds

❌ Wrong:
```izilang
var arr = [1, 2, 3];
print(arr[5]);  // Error: out of bounds
```

✅ Correct:
```izilang
var arr = [1, 2, 3];
if (5 < len(arr)) {
    print(arr[5]);
}
```

---

### 4. Function Arguments

❌ Wrong:
```izilang
fn greet(name) {
    print("Hello, " + name);
}
greet();  // Error: missing argument
```

✅ Correct:
```izilang
fn greet(name) {
    print("Hello, " + name);
}
greet("Alice");  // Provide required argument
```

---

## See Also

- [Getting Started Guide](GETTING_STARTED.md)
- [CLI Reference](CLI_REFERENCE.md)
- [Language Specification](V03_SPECIFICATION.md)
- [GitHub Issues](https://github.com/ismobaga/izilang/issues)

---

**IziLang** - A modern, expressive programming language

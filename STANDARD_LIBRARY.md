# IziLang Standard Library

The IziLang standard library provides essential functionality for common programming tasks through native functions implemented in C++.

## Overview

IziLang now supports a **modular standard library system**. You can import modules in three ways:

1. **Simple import**: `import "math";` - Creates a module object (e.g., `math.sqrt(16)`)
2. **Named imports**: `import { sqrt, pi } from "math";` - Import specific functions/constants
3. **Wildcard import**: `import * as m from "math";` - Import module with custom alias

All standard library functions remain available globally for backward compatibility, but the modular approach is recommended for better code organization.

## math - Mathematical Functions

Mathematical constants and functions for numerical computations.

### Module Import

```izilang
import "math";
var result = math.sqrt(16);  // 4
var pi_value = math.pi;      // 3.14159...
```

Or import specific items:
```izilang
import { sqrt, sin, cos, pi } from "math";
var result = sqrt(16);
```

### Constants

- **pi** / **PI** = 3.14159265358979... - The mathematical constant π
- **e** / **E** = 2.71828182845905... - Euler's number

### Functions

**Power and Root:**
- `sqrt(x)` - Square root (x must be non-negative)
- `pow(base, exp)` - Raise base to the power of exp

**Rounding:**
- `floor(x)` - Round down to nearest integer
- `ceil(x)` - Round up to nearest integer
- `round(x)` - Round to nearest integer
- `abs(x)` - Absolute value

**Trigonometry (angles in radians):**
- `sin(x)` - Sine function
- `cos(x)` - Cosine function  
- `tan(x)` - Tangent function

**Min/Max:**
- `min(...values)` - Return smallest value (variadic)
- `max(...values)` - Return largest value (variadic)

### Example

```izilang
import "math";

print("math.pi =", math.pi);
print("math.sqrt(16) =", math.sqrt(16));
print("math.pow(2, 3) =", math.pow(2, 3));
print("math.sin(math.pi/2) =", math.sin(math.pi / 2));
print("math.min(1, 2, 3) =", math.min(1, 2, 3));
```

Or with named imports:
```izilang
import { sqrt, sin, pi } from "math";

print("pi =", pi);
print("sqrt(16) =", sqrt(16));
print("sin(pi/2) =", sin(pi / 2));
```

## string - String Manipulation

Functions for working with strings.

### Module Import

```izilang
import "string";
var upper = string.toUpper("hello");  // "HELLO"
```

Or import specific functions:
```izilang
import { toUpper, split, trim } from "string";
var upper = toUpper("hello");
```

### Functions

**Extraction:**
- `substring(str, start, length?)` - Extract substring (length optional, defaults to end of string)
- `split(str, delimiter)` - Split string into array (empty delimiter splits into characters)

**Case Conversion:**
- `toUpper(str)` - Convert to uppercase
- `toLower(str)` - Convert to lowercase

**Whitespace:**
- `trim(str)` - Remove leading and trailing whitespace

**Search and Replace:**
- `replace(str, from, to)` - Replace all occurrences of 'from' with 'to'
- `indexOf(str, substring)` - Find index of substring (-1 if not found)
- `startsWith(str, prefix)` - Check if string starts with prefix (returns boolean)
- `endsWith(str, suffix)` - Check if string ends with suffix (returns boolean)

**Array Conversion:**
- `join(array, separator)` - Join array elements into string with separator

### Example

```izilang
import "string";

var str = "Hello, World!";
print(string.substring(str, 0, 5));      // "Hello"
print(string.toUpper(str));              // "HELLO, WORLD!"
print(string.split(str, ", "));          // ["Hello", "World!"]
print(string.replace(str, "World", "IziLang"));  // "Hello, IziLang!"
```

## array - Array Utilities

Functions for array manipulation and functional programming.

### Module Import

```izilang
import "array";
var doubled = array.map([1, 2, 3], fn(x) { return x * 2; });
```

Or import specific functions:
```izilang
import { map, filter, reduce } from "array";
```

### Functions

**Higher-Order Functions:**
- `map(array, fn)` - Apply function to each element, return new array
- `filter(array, fn)` - Filter elements by predicate function, return new array
- `reduce(array, fn, initial?)` - Reduce array to single value (initial value optional)

**Array Manipulation:**
- `sort(array)` - Sort array (returns new sorted array, works with numbers and strings)
- `reverse(array)` - Reverse array (returns new reversed array)
- `concat(array1, array2)` - Concatenate two arrays
- `slice(array, start, end?)` - Extract subarray (end optional, defaults to array length)

**Basic Operations:**
- `push(array, element)` - Add element to end (mutates array, returns array)
- `pop(array)` - Remove and return last element (mutates array)
- `len(array)` - Get array length

### Example

```izilang
import { map, filter, reduce } from "array";

var numbers = [1, 2, 3, 4, 5];

fn double(x) {
    return x * 2;
}
print(map(numbers, double));  // [2, 4, 6, 8, 10]

fn isEven(x) {
    return x - floor(x / 2) * 2 == 0;
}
print(filter(numbers, isEven));  // [2, 4]

fn add(a, b) {
    return a + b;
}
print(reduce(numbers, add, 0));  // 15
```

## io - File I/O

Functions for file operations.

### Module Import

```izilang
import "io";
io.writeFile("/tmp/test.txt", "Hello!");
var content = io.readFile("/tmp/test.txt");
```

Or import specific functions:
```izilang
import { readFile, writeFile, exists } from "io";
```

### Functions

**File Operations:**
- `readFile(filename)` - Read entire file as string
- `writeFile(filename, content)` - Write string to file (overwrites existing file)
- `appendFile(filename, content)` - Append string to file
- `fileExists(filename)` / `exists(filename)` - Check if file exists (returns boolean)

**Console:**
- `print(...values)` - Print values to console (variadic, values separated by spaces)

### Example

```izilang
import "io";

io.writeFile("/tmp/test.txt", "Hello, World!");

if (io.exists("/tmp/test.txt")) {
    var content = io.readFile("/tmp/test.txt");
    print(content);  // "Hello, World!"
}

io.appendFile("/tmp/test.txt", "\nNew line added.");
```

## assert - Runtime Assertions

Assertion helpers for defensive programming and testing.

### Module Import

```izilang
import "assert";
assert.ok(x > 0);
```

Or use the namespaced version:

```izilang
import * as assert from "std.assert";
assert.ok(x > 0);
assert.eq(a, b);
assert.ne(a, b);
```

### Functions

**Assertion Functions:**
- `ok(condition, message?)` - Assert that a condition is truthy. Throws an error if condition is falsy. Optional custom error message.
- `eq(a, b)` - Assert that two values are equal. Throws an error if values are not equal.
- `ne(a, b)` - Assert that two values are not equal. Throws an error if values are equal.

### Example

```izilang
import * as assert from "std.assert";

// Basic assertions
assert.ok(true);
assert.ok(5 > 3);
assert.ok(x !== nil, "x should not be nil");

// Equality assertions
assert.eq(1 + 1, 2);
assert.eq("hello", greeting);

// Inequality assertions
assert.ne(a, b);
assert.ne(true, false);

// Assertions throw errors on failure
try {
    assert.ok(false, "This will fail");
} catch(e) {
    print("Caught assertion error:", e);
}
```

## json - JSON Parsing (Placeholder)

JSON parsing and generation module. *Coming soon.*

### Module Import

```izilang
import "json";
```

## http - HTTP Client (Placeholder)

Basic HTTP client functionality. *Coming soon.*

### Module Import

```izilang
import "http";
```

## Additional Built-in Functions

These functions are available globally without imports:

**Utility:**
- `clock()` - Get current time in seconds since epoch (as floating point)
- `len(value)` - Get length of string, array, or map

**Map Operations:**
- `keys(map)` - Get array of map keys
- `values(map)` - Get array of map values
- `hasKey(map, key)` - Check if map has key (returns boolean)

## Notes

- **Module System**: All standard library modules can be imported using `import "moduleName"`, `import { item } from "moduleName"`, or `import * as alias from "moduleName"`
- **Backward Compatibility**: All functions remain available globally without imports for backward compatibility
- Functions are implemented as native C++ code for performance
- Array functions (map, filter, reduce, sort, reverse, concat, slice) return new arrays and don't mutate originals
- Exception: `push` and `pop` mutate the array in-place
- String functions return new strings (strings are immutable)
- File I/O functions use POSIX `stat` for `fileExists()`, which may have platform limitations on Windows

## Known Limitations

- The `fileExists()` function uses `<sys/stat.h>` which is POSIX-specific and may not work on all platforms
- JSON and HTTP modules are placeholders for future implementation

## Module System Examples

### Three Ways to Import

```izilang
import "math";
var result1 = math.sqrt(16);

import { sqrt, pi } from "math";
var result2 = sqrt(25);

import * as m from "math";
var result3 = m.sqrt(36);
```

### Combining Multiple Modules

```izilang
import "math";
import "string";
import { map, filter } from "array";

var nums = [1, 2, 3, 4];
var doubled = map(nums, fn(x) { return x * 2; });
var text = string.join(doubled, ", ");
print(text);  // "2, 4, 6, 8"
```

## Running Tests

Test files are provided:

```bash
./bin/Debug/izi/izi test_modules.iz
```

Or run the integrated test suite:

```bash
./bin/Debug/tests/tests
```

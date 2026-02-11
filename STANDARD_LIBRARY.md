# IziLang Standard Library

The IziLang standard library provides essential functionality for common programming tasks through native functions implemented in C++.

## Overview

All standard library functions are registered as native functions and are available globally without requiring any imports. The `std/math.iz` module provides mathematical constants that can be imported when needed.

## std.math - Mathematical Functions

Mathematical constants and functions for numerical computations.

### Constants (requires `import "std/math.iz"`)

- **PI** = 3.14159265 - The mathematical constant π
- **E** = 2.71828183 - Euler's number

### Functions (available globally)

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
import "std/math.iz";

print("PI =", PI);
print("sqrt(16) =", sqrt(16));
print("pow(2, 3) =", pow(2, 3));
print("sin(PI/2) =", sin(PI / 2));
print("min(1, 2, 3) =", min(1, 2, 3));
```

## std.string - String Manipulation

Functions for working with strings.

### Functions (available globally)

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
var str = "Hello, World!";
print(substring(str, 0, 5));
print(toUpper(str));
print(split(str, ", "));
print(replace(str, "World", "IziLang"));
```

## std.array - Array Utilities

Functions for array manipulation and functional programming.

### Functions (available globally)

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
var numbers = [1, 2, 3, 4, 5];

fn double(x) {
    return x * 2;
}
print(map(numbers, double));

fn isEven(x) {
    return x - floor(x / 2) * 2 == 0;
}
print(filter(numbers, isEven));

fn add(a, b) {
    return a + b;
}
print(reduce(numbers, add, 0));
```

## std.io - File I/O

Functions for file operations.

### Functions (available globally)

**File Operations:**
- `readFile(filename)` - Read entire file as string
- `writeFile(filename, content)` - Write string to file (overwrites existing file)
- `appendFile(filename, content)` - Append string to file
- `fileExists(filename)` - Check if file exists (returns boolean)

**Console:**
- `print(...values)` - Print values to console (variadic, values separated by spaces)

### Example

```izilang
writeFile("/tmp/test.txt", "Hello, World!");

if (fileExists("/tmp/test.txt")) {
    var content = readFile("/tmp/test.txt");
    print(content);
}

appendFile("/tmp/test.txt", "
New line added.");
```

## Additional Built-in Functions

**Utility:**
- `clock()` - Get current time in seconds since epoch (as floating point)
- `len(value)` - Get length of string, array, or map

**Map Operations:**
- `keys(map)` - Get array of map keys
- `values(map)` - Get array of map values
- `hasKey(map, key)` - Check if map has key (returns boolean)

## Notes

- All functions except those in `std/math.iz` are available globally without imports
- Functions are implemented as native C++ code for performance
- Array functions (map, filter, reduce, sort, reverse, concat, slice) return new arrays and don't mutate originals
- Exception: `push` and `pop` mutate the array in-place
- String functions return new strings (strings are immutable)

## Running Tests

Test files are provided for each module:

```bash
./bin/Debug/izi/izi test_std_math.iz
./bin/Debug/izi/izi test_std_string.iz
./bin/Debug/izi/izi test_std_array.iz
./bin/Debug/izi/izi test_std_io.iz
```

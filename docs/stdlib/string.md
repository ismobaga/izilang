# string — String Manipulation

The `string` module provides functions for working with strings.

## Import

```izilang
// Module object
import "string";
var upper = string.toUpper("hello");  // "HELLO"

// Named imports
import { toUpper, split, trim } from "string";
var upper = toUpper("hello");
```

## Functions

### Extraction

#### `substring(str, start, length?)`

Returns a substring of `str` starting at index `start`. If `length` is omitted, returns the remainder of the string.

```izilang
import { substring } from "string";
print(substring("Hello, World!", 0, 5));  // "Hello"
print(substring("Hello, World!", 7));     // "World!"
```

#### `split(str, delimiter)`

Splits `str` into an array using `delimiter`. If `delimiter` is an empty string `""`, the string is split into individual characters.

```izilang
import { split } from "string";
print(split("a,b,c", ","));   // ["a", "b", "c"]
print(split("hi", ""));       // ["h", "i"]
```

### Case Conversion

#### `toUpper(str)`

Returns `str` converted to uppercase.

```izilang
print(string.toUpper("hello"));  // "HELLO"
```

#### `toLower(str)`

Returns `str` converted to lowercase.

```izilang
print(string.toLower("HELLO"));  // "hello"
```

### Whitespace

#### `trim(str)`

Removes leading and trailing whitespace from `str`.

```izilang
print(string.trim("  hello  "));  // "hello"
```

### Search and Replace

#### `replace(str, from, to)`

Replaces **all** occurrences of `from` in `str` with `to`.

```izilang
print(string.replace("aabbcc", "b", "X"));  // "aaXXcc"
```

#### `indexOf(str, substring)`

Returns the index of the first occurrence of `substring` in `str`, or `-1` if not found.

```izilang
print(string.indexOf("hello", "ll"));   // 2
print(string.indexOf("hello", "xyz"));  // -1
```

#### `startsWith(str, prefix)`

Returns `true` if `str` starts with `prefix`.

```izilang
print(string.startsWith("hello", "he"));  // true
print(string.startsWith("hello", "lo"));  // false
```

#### `endsWith(str, suffix)`

Returns `true` if `str` ends with `suffix`.

```izilang
print(string.endsWith("hello", "lo"));  // true
print(string.endsWith("hello", "he"));  // false
```

### Array Conversion

#### `join(array, separator)`

Joins the elements of `array` into a single string, separated by `separator`.

```izilang
import { join } from "string";
print(join(["a", "b", "c"], "-"));   // "a-b-c"
print(join([1, 2, 3], ", "));        // "1, 2, 3"
```

## Complete Example

```izilang
import "string";

var s = "  Hello, World!  ";

// Whitespace
print(string.trim(s));                   // "Hello, World!"

// Case conversion
print(string.toUpper("hello"));          // "HELLO"
print(string.toLower("WORLD"));          // "world"

// Extraction
print(string.substring(s, 2, 5));        // "Hello"

// Search
print(string.indexOf(s, "World"));       // 9
print(string.startsWith(s, "  Hello"));  // true
print(string.endsWith(s, "!  "));        // true

// Replace
print(string.replace(s, "World", "IziLang"));
// "  Hello, IziLang!  "

// Split and Join
var parts = string.split("apple,banana,cherry", ",");
print(parts);                            // [apple, banana, cherry]
print(string.join(parts, " | "));        // "apple | banana | cherry"
```

## Notes

- Strings are **immutable** — all functions return new strings.
- Indices are **zero-based**.
- `split`, `substring`, and `join` work with any value type in arrays (elements are converted to strings via `toString()`).

## See Also

- [Standard Library Index](README.md)
- [array module](array.md)

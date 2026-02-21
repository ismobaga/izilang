# regex — Regular Expressions

The `regex` module provides pattern matching and string replacement using regular expressions.

## Import

```izilang
import * as regex from "std.regex";
var found = regex.test("Hello World", "World");

// Named imports
import { test, replace } from "std.regex";
```

## Functions

### `test(text, pattern)`

Returns `true` if `pattern` matches anywhere in `text`, `false` otherwise.

```izilang
import * as regex from "std.regex";

print(regex.test("Hello, World!", "World"));   // true
print(regex.test("Hello, World!", "xyz"));     // false
print(regex.test("abc123", "\\d+"));           // true
print(regex.test("HELLO", "(?i)hello"));       // true (case-insensitive)
```

### `replace(text, pattern, replacement)`

Replaces **all** matches of `pattern` in `text` with `replacement`. Supports backreferences (`$1`, `$2`, …) for capture groups.

```izilang
import * as regex from "std.regex";

// Simple replacement
print(regex.replace("Hello, World!", "World", "IziLang"));
// "Hello, IziLang!"

// Replace all digits
print(regex.replace("abc123def456", "\\d+", "X"));
// "abcXdefX"

// Reformat a date with capture groups
print(regex.replace("2026-03-15", "(\\d{4})-(\\d{2})-(\\d{2})", "$3/$2/$1"));
// "15/03/2026"
```

> **Note**: `regex.replace` replaces **all** occurrences, not just the first.

## Pattern Syntax

IziLang uses **C++11 ECMAScript** regex syntax (similar to JavaScript):

| Syntax | Description |
|--------|-------------|
| `.` | Any character except newline |
| `*` | Match 0 or more (greedy) |
| `+` | Match 1 or more (greedy) |
| `?` | Match 0 or 1 |
| `^` | Start of string |
| `$` | End of string |
| `[abc]` | Character class |
| `[^abc]` | Negated character class |
| `\d` | Digit — use `"\\d"` in string literals |
| `\w` | Word character — use `"\\w"` in string literals |
| `\s` | Whitespace — use `"\\s"` in string literals |
| `(...)` | Capture group |
| `(?i)` | Case-insensitive flag (inline) |

## Complete Example

```izilang
import * as regex from "std.regex";

var log = "2026-03-15 14:30:00 ERROR Something went wrong at line 42";

// Check for errors
if (regex.test(log, "ERROR")) {
    print("Error found in log");
}

// Extract the date portion by replacing everything after it
var date = regex.replace(log, "^(\\d{4}-\\d{2}-\\d{2}).*", "$1");
print("Date:", date);  // "2026-03-15"

// Redact line numbers
var sanitized = regex.replace(log, "line \\d+", "line [REDACTED]");
print(sanitized);

// Validate an email-like pattern
fn isEmail(s) {
    return regex.test(s, "^[\\w.+-]+@[\\w-]+\\.[\\w.]+$");
}
print(isEmail("user@example.com"));  // true
print(isEmail("not-an-email"));      // false
```

## Known Limitations

- `regex.match()` is currently **disabled** due to a segmentation fault bug. Calling it throws a runtime error.
- Use `regex.test()` to check for a match and `regex.replace()` for extraction via capture groups.

## See Also

- [Standard Library Index](README.md)
- [string module](string.md) — Simple string search without regex overhead

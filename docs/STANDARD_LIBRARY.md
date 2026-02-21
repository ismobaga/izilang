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

## env - Environment Variables

Access and manipulate environment variables in a cross-platform manner.

### Module Import

```izilang
import * as env from "std.env";
var home = env.get("HOME");
env.set("MODE", "production");
var hasDebug = env.exists("DEBUG");
```

Or import specific functions:
```izilang
import { get, set, exists } from "std.env";
var path = get("PATH");
```

### Functions

**Environment Variable Access:**
- `get(name: String): String | Nil` - Get an environment variable by name. Returns the value as a string, or `nil` if the variable doesn't exist.
- `set(name: String, value: String): Nil` - Set an environment variable to a specific value. Both arguments must be strings.
- `exists(name: String): Bool` - Check if an environment variable exists. Returns `true` if it exists, `false` otherwise.

### Example

```izilang
import * as env from "std.env";

// Get environment variables
var home = env.get("HOME");
var path = env.get("PATH");
var user = env.get("USER");

if (home != nil) {
    print("Home directory:", home);
}

// Set environment variables
env.set("APP_MODE", "production");
env.set("DEBUG_LEVEL", "2");

// Check if variables exist
if (env.exists("DEBUG")) {
    print("Debug mode is enabled");
}

// Set and retrieve
env.set("MY_VAR", "my_value");
var value = env.get("MY_VAR");
print("MY_VAR =", value);  // "my_value"

// Non-existent variables return nil
var unknown = env.get("NON_EXISTENT_VAR");
if (unknown == nil) {
    print("Variable doesn't exist");
}
```

### Platform Notes

- Uses POSIX `getenv()` for reading (available on all platforms)
- Uses `setenv()` on Linux/macOS for writing
- Uses `_putenv_s()` on Windows for writing
- Environment variable changes only affect the current process and child processes
- Variable names and values are case-sensitive on Linux/macOS, case-insensitive on Windows

## time - Time and Sleep Operations

Time-related functions for getting current time, sleeping, and formatting timestamps.

### Module Import

```izilang
import * as time from "std.time";
var now = time.now();
time.sleep(1.5);
```

Or import specific functions:
```izilang
import { now, sleep, format } from "std.time";
```

### Functions

**Time Operations:**
- `now(): Number` - Get current Unix timestamp in seconds (as floating point with millisecond precision)
- `sleep(seconds: Number): Nil` - Sleep for the specified number of seconds (supports fractional seconds)
- `format(timestamp: Number, format?: String): String` - Format a Unix timestamp as a human-readable string

### Format Specifiers

The `format()` function accepts standard C `strftime` format specifiers:

- `%Y` - 4-digit year (e.g., 2026)
- `%m` - Month (01-12)
- `%d` - Day of month (01-31)
- `%H` - Hour (00-23)
- `%M` - Minute (00-59)
- `%S` - Second (00-59)
- Default format: `"%Y-%m-%d %H:%M:%S"`

### Example

```izilang
import * as time from "std.time";

// Get current time
var now = time.now();
print("Current timestamp:", now);  // e.g., 1708635123.456

// Format with default format
var formatted = time.format(now);
print("Formatted:", formatted);  // "2026-02-12 21:15:23"

// Custom format
var dateOnly = time.format(now, "%Y-%m-%d");
print("Date:", dateOnly);  // "2026-02-12"

var timeOnly = time.format(now, "%H:%M:%S");
print("Time:", timeOnly);  // "21:15:23"

// Sleep for 1.5 seconds
print("Sleeping...");
time.sleep(1.5);
print("Done!");
```

## json - JSON Parsing and Serialization

Parse JSON strings into IziLang values and serialize IziLang values to JSON strings.

### Module Import

```izilang
import * as json from "std.json";
var data = json.parse('{"name": "IziLang"}');
var jsonStr = json.stringify(data);
```

Or import specific functions:
```izilang
import { parse, stringify } from "std.json";
```

### Functions

**JSON Operations:**
- `parse(jsonString: String): Any` - Parse a JSON string into an IziLang value
- `stringify(value: Any): String` - Convert an IziLang value to a JSON string

### Supported Types

| IziLang Type | JSON Type | Example |
|--------------|-----------|---------|
| `nil` | `null` | `nil` → `"null"` |
| `bool` | `boolean` | `true` → `"true"` |
| `double` | `number` | `42.5` → `"42.5"` |
| `string` | `string` | `"hello"` → `"\"hello\""` |
| `Array` | `array` | `[1, 2]` → `"[1,2]"` |
| `Map` | `object` | `{a: 1}` → `"{\"a\":1}"` |

### Example

```izilang
import * as json from "std.json";

// Parse JSON
var numJson = "42";
var num = json.parse(numJson);
print(num);  // 42

var arrayJson = '[1, 2, 3, "four"]';
var arr = json.parse(arrayJson);
print(arr);  // [1, 2, 3, four]

var objJson = '{"name": "IziLang", "version": 0.3}';
var obj = json.parse(objJson);
print(obj);  // {name: IziLang, version: 0.3}

// Stringify values
print(json.stringify(nil));  // "null"
print(json.stringify(true));  // "true"
print(json.stringify(42));  // "42"
print(json.stringify("hello"));  // "\"hello\""
print(json.stringify([1, 2, 3]));  // "[1,2,3]"
print(json.stringify({"a": 1, "b": 2}));  // "{\"a\":1,\"b\":2}"

// Round-trip conversion
var data = {"users": ["alice", "bob"], "count": 2};
var jsonStr = json.stringify(data);
var parsed = json.parse(jsonStr);
print(parsed);  // {users: [alice, bob], count: 2}
```

### Notes

- JSON parsing is strict and will throw an error for invalid JSON
- NaN and Infinity numbers are converted to `null` during stringify
- Object keys must be strings in JSON
- Circular references are not supported and will cause undefined behavior

## regex - Regular Expression Operations

Pattern matching and string replacement using regular expressions.

### Module Import

```izilang
import * as regex from "std.regex";
var hasMatch = regex.test("Hello World", "World");
```

Or import specific functions:
```izilang
import { test, replace } from "std.regex";
```

### Functions

**Regex Operations:**
- `test(text: String, pattern: String): Bool` - Test if a pattern matches anywhere in the text
- `replace(text: String, pattern: String, replacement: String): String` - Replace all matches of pattern with replacement
- ~~`match(text: String, pattern: String): Array | Nil`~~ - **Currently disabled due to a bug**

### Pattern Syntax

IziLang uses C++11 ECMAScript regex syntax:

- `.` - Match any character
- `*` - Match 0 or more
- `+` - Match 1 or more
- `?` - Match 0 or 1
- `^` - Start of string
- `$` - End of string
- `[abc]` - Character class
- `[^abc]` - Negated character class
- `\d` - Digit (use `\\d` in string literals)
- `\w` - Word character (use `\\w` in string literals)
- `\s` - Whitespace (use `\\s` in string literals)
- `(...)` - Capture group

### Example

```izilang
import * as regex from "std.regex";

// Test for pattern existence
var text = "Hello, World! Welcome to IziLang.";
print(regex.test(text, "World"));  // true
print(regex.test(text, "xyz"));    // false

// Case-insensitive test (use (?i) flag in pattern)
print(regex.test(text, "(?i)world"));  // true

// Replace patterns
var replaced = regex.replace(text, "World", "Universe");
print(replaced);  // "Hello, Universe! Welcome to IziLang."

// Replace all digits
var withDigits = "abc123def456";
var noDigits = regex.replace(withDigits, "\\d+", "X");
print(noDigits);  // "abcXdefX"

// Remove all spaces
var spaced = "a b c d";
var compact = regex.replace(spaced, " ", "");
print(compact);  // "abcd"

// Replace with capture groups
var dated = "Date: 2026-02-12";
var reformatted = regex.replace(dated, "(\\d{4})-(\\d{2})-(\\d{2})", "$3/$2/$1");
print(reformatted);  // "Date: 12/02/2026"
```

### Notes

- Regex patterns use C++11 ECMAScript syntax (similar to JavaScript)
- Backslashes in patterns must be escaped in string literals (e.g., `"\\d+"` for digits)
- `replace()` replaces **all** matches, not just the first one
- `match()` is currently disabled due to a segmentation fault bug and will throw an error if called

## http - HTTP Client

Synchronous HTTP client for making HTTP/1.1 requests. Supports GET, POST, and generic requests via a plain TCP socket connection. HTTPS is not supported in this version.

### Module Import

```izilang
import "http";
var response = http.get("http://example.com/api/data");
```

Or use the namespaced version:

```izilang
import * as http from "std.http";
var response = http.get("http://example.com/");
```

Or import specific functions:
```izilang
import { get, post, request } from "http";
```

### Functions

**Request Functions:**
- `get(url: String): Map` - Make an HTTP GET request to the given URL.
- `post(url: String, body: String, contentType?: String): Map` - Make an HTTP POST request. `contentType` defaults to `"application/x-www-form-urlencoded"`.
- `request(options: Map): Map` - Make a generic HTTP request. The `options` map must contain a `"url"` string key and may contain `"method"` (default `"GET"`), `"body"`, and `"contentType"`.

### Response Map

All three functions return a response map with the following fields:

| Field | Type | Description |
|-------|------|-------------|
| `status` | Number | HTTP status code (e.g., `200`, `404`) |
| `statusText` | String | HTTP status text (e.g., `"OK"`, `"Not Found"`) |
| `ok` | Bool | `true` if status code is in the 200–299 range |
| `body` | String | Response body as a string |
| `headers` | Map | Response headers (key → value) |

### Example

```izilang
import * as http from "http";

// Simple GET request
var response = http.get("http://httpbin.org/get");
if (response.ok) {
    print("Status:", response.status);
    print("Body:", response.body);
} else {
    print("Request failed:", response.status, response.statusText);
}

// POST request with a body
var postResponse = http.post(
    "http://httpbin.org/post",
    "name=IziLang&version=0.3",
    "application/x-www-form-urlencoded"
);
print("POST status:", postResponse.status);

// Generic request
var putResponse = http.request({
    "method": "PUT",
    "url": "http://httpbin.org/put",
    "body": "{\"key\": \"value\"}",
    "contentType": "application/json"
});
print("PUT status:", putResponse.status);

// Error handling
try {
    var res = http.get("http://httpbin.org/status/500");
    if (!res.ok) {
        print("Server error:", res.status);
    }
} catch (e) {
    print("Network error:", e);
}
```

### Notes

- Only plain **HTTP** is supported. HTTPS connections will throw an error.
- Requests are **synchronous** (blocking) — the program waits for the response before continuing.
- All unsupported URL schemes (e.g., `ftp://`, `https://`) throw a runtime error.
- DNS resolution failures or connection refused errors also throw a runtime error.
- The `Connection: close` header is sent automatically; persistent connections are not supported.

## ui - Window Library

The `ui` module provides a graphical window and drawing API backed by [raylib](https://www.raylib.com/). It enables the creation of interactive GUI applications directly from IziLang scripts.

> **Note**: The `ui` module requires raylib. Build the interpreter with `--raylib=<path>` passed to `premake5` to enable full functionality. Without raylib, the module can be imported and constants/color values work, but calling window-creation or drawing functions will throw a runtime error.

### Module Import

```izilang
import ui

var win = ui.createWindow("My App", 800, 600)
while win.isOpen() {
    if ui.keyPressed(ui.key.escape) { win.close() }
    win.beginDrawing()
    win.clear(ui.color(20, 20, 24))
    win.drawText(20, 20, "Hello IZI", 24, ui.color(240, 240, 240))
    win.endDrawing()
}
```

Or with an alias:
```izilang
import * as ui from "std.ui";
```

### Window Creation

- `ui.createWindow(title, width, height)` — Open a new OS window and return a `Window` object. Only one window can be open at a time.

### Window Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `isOpen` | `() -> bool` | Returns `true` while the window has not been closed |
| `close` | `()` | Close the window |
| `beginDrawing` | `()` | Begin a render frame |
| `endDrawing` | `()` | End the render frame and present it on screen |
| `clear` | `(color)` | Clear the background with the given color |
| `setTitle` | `(title)` | Change the window title |
| `getSize` | `() -> {width, height}` | Return current window dimensions |
| `getFps` | `() -> number` | Return current frames per second |
| `drawText` | `(x, y, text, fontSize, color)` | Draw text at window coordinates |
| `fillRect` | `(x, y, w, h, color)` | Draw a filled rectangle |
| `drawRect` | `(x, y, w, h, color)` | Draw a rectangle outline |
| `drawLine` | `(x1, y1, x2, y2, thickness, color)` | Draw a line |
| `drawCircle` | `(x, y, radius, color)` | Draw a filled circle |
| `createPanel` | `(x, y, w, h) -> Panel` | Create a scissor-clipped sub-region |

### Panel Methods

Panels are virtual sub-regions of the window. All coordinate arguments are **panel-local** (relative to the panel's top-left corner).

| Method | Signature | Description |
|--------|-----------|-------------|
| `begin` | `()` | Activate scissor clipping for this panel |
| `end` | `()` | Deactivate scissor clipping |
| `getMousePosition` | `() -> {x, y}` | Mouse position in panel-local coordinates |
| `containsMouse` | `() -> bool` | `true` when the mouse cursor is inside this panel |
| `drawText` | `(x, y, text, fontSize, color)` | Draw text in panel-local coordinates |
| `fillRect` | `(x, y, w, h, color)` | Draw a filled rectangle in panel-local coordinates |
| `drawRect` | `(x, y, w, h, color)` | Draw a rectangle outline in panel-local coordinates |
| `drawLine` | `(x1, y1, x2, y2, thickness, color)` | Draw a line in panel-local coordinates |
| `drawCircle` | `(x, y, radius, color)` | Draw a filled circle in panel-local coordinates |

### Colors

- `ui.color(r, g, b)` — Create a color value with full opacity (alpha = 255).
- `ui.color(r, g, b, a)` — Create a color value with a custom alpha (0–255).

Colors are map values with `r`, `g`, `b`, `a` fields.

### Keyboard Input

- `ui.keyDown(key)` — Returns `true` while the key is held down.
- `ui.keyPressed(key)` — Returns `true` on the frame the key was first pressed.
- `ui.getCharPressed()` — Returns the Unicode codepoint of the last character pressed (0 if none).

**Key constants** (`ui.key.*`):

| Constant | Description |
|----------|-------------|
| `escape` | Escape key |
| `enter` | Enter / Return key |
| `space` | Space bar |
| `left` | Left arrow |
| `right` | Right arrow |
| `up` | Up arrow |
| `down` | Down arrow |
| `a`–`d`, `s`, `w` | Letter keys |

### Mouse Input

- `ui.mouseDown(button)` — Returns `true` while the mouse button is held.
- `ui.mousePressed(button)` — Returns `true` on the frame the button was first pressed.
- `ui.getMousePosition()` — Returns a map `{x, y}` with the current cursor position.
- `ui.getMouseWheelMove()` — Returns the mouse wheel scroll delta for the current frame.

**Mouse button constants** (`ui.mouse.*`): `left`, `right`, `middle`.

### Full Example

```izilang
import ui

win = ui.createWindow("IZI + raylib", 900, 600)

while win.isOpen() {
  if ui.keyPressed(ui.key.escape) { win.close() }

  win.beginDrawing()
  win.clear(ui.color(20, 20, 24))

  pos = ui.getMousePosition()
  win.fillRect(pos.x - 50, pos.y - 20, 100, 40, ui.color(80, 140, 255))

  win.drawText(20, 20, "Hello IZI", 24, ui.color(240, 240, 240))
  win.drawCircle(450, 300, 40, ui.color(255, 80, 80))

  fps = win.getFps()
  win.drawText(20, 570, "FPS: " + str(fps), 14, ui.color(120, 120, 120))

  win.endDrawing()
}
```

See also: [`examples/demo_ui.iz`](../examples/demo_ui.iz) and [`examples/ui_panels.izi`](../examples/ui_panels.izi).

### Building with raylib

```bash
premake5 gmake --raylib=/path/to/raylib
make config=debug
./bin/Debug/izi/izi examples/demo_ui.iz
```

On Linux, the raylib build also links `GL` and `X11` automatically.

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
- HTTP module supports plain HTTP only; HTTPS requires SSL/TLS and is not yet supported
- `regex.match()` is temporarily disabled due to a segmentation fault issue; use `regex.test()` and `regex.replace()` instead

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

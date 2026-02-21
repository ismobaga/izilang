# IziLang Standard Library Reference

The IziLang standard library provides essential functionality for common programming tasks through native functions implemented in C++.

## Import Styles

IziLang supports three ways to import standard library modules:

```izilang
// 1. Simple import — use as a namespace object
import "math";
var result = math.sqrt(16);

// 2. Named imports — bring specific names into scope
import { sqrt, pi } from "math";
var result = sqrt(16);

// 3. Wildcard import — import all exports under an alias
import * as m from "math";
var result = m.sqrt(16);
```

All standard library functions are also available **globally** without any import for backward compatibility.

## Modules

| Module | Import Path | Description |
|--------|-------------|-------------|
| [math](math.md) | `"math"` | Mathematical constants and functions |
| [string](string.md) | `"string"` | String manipulation functions |
| [array](array.md) | `"array"` | Array utilities and higher-order functions |
| [io](io.md) | `"io"` | File I/O and console output |
| [assert](assert.md) | `"std.assert"` | Assertions for testing and validation |
| [env](env.md) | `"std.env"` | Environment variable access |
| [time](time.md) | `"std.time"` | Time, sleep, and formatting |
| [json](json.md) | `"std.json"` | JSON parsing and serialization |
| [regex](regex.md) | `"std.regex"` | Regular expression matching and replacement |
| [http](http.md) | `"http"` / `"std.http"` | Synchronous HTTP client |
| [ui](ui.md) | `"ui"` / `"std.ui"` | Graphical window and drawing (requires raylib) |
| [ipc](ipc.md) | `"std.ipc"` | Inter-process communication via named pipes |

## Global Built-ins

These functions are always available without any import:

| Function | Description |
|----------|-------------|
| `print(...values)` | Print values to stdout, space-separated |
| `len(value)` | Length of a string, array, or map |
| `clock()` | Current Unix timestamp in seconds (float) |
| `typeOf(value)` | Type name of a value as a string |
| `toString(value)` | Convert value to its string representation |
| `toNumber(str)` | Parse a string as a number |
| `keys(map)` | Array of all keys in a map |
| `values(map)` | Array of all values in a map |
| `hasKey(map, key)` | `true` if map contains the given key |
| `push(array, value)` | Append a value to an array (mutates in place) |
| `pop(array)` | Remove and return the last element (mutates) |

## See Also

- [Language Tutorial](../TUTORIAL.md) — Learn IziLang from scratch
- [CLI Reference](../CLI_REFERENCE.md) — Command-line interface
- [Concurrency Guide](../CONCURRENCY.md) — Async/await and threads

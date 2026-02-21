# io — File I/O

The `io` module provides functions for reading and writing files.

## Import

```izilang
// Module object
import "io";
io.writeFile("/tmp/test.txt", "Hello!");
var content = io.readFile("/tmp/test.txt");

// Named imports
import { readFile, writeFile, exists } from "io";
```

## Functions

### `readFile(filename)`

Reads the entire contents of `filename` and returns it as a string. Throws a runtime error if the file does not exist or cannot be read.

```izilang
import "io";
var content = io.readFile("/tmp/hello.txt");
print(content);
```

### `writeFile(filename, content)`

Writes `content` to `filename`, **overwriting** any existing file. Creates the file if it does not exist.

```izilang
import "io";
io.writeFile("/tmp/hello.txt", "Hello, World!");
```

### `appendFile(filename, content)`

Appends `content` to the end of `filename`. Creates the file if it does not exist.

```izilang
import "io";
io.appendFile("/tmp/log.txt", "Line 1\n");
io.appendFile("/tmp/log.txt", "Line 2\n");
```

### `fileExists(filename)` / `exists(filename)`

Returns `true` if the file at `filename` exists, `false` otherwise. Both `fileExists` and `exists` are aliases for the same function.

```izilang
import "io";
if (io.exists("/tmp/data.txt")) {
    var data = io.readFile("/tmp/data.txt");
    print(data);
} else {
    print("File not found");
}
```

## Complete Example

```izilang
import "io";

var path = "/tmp/izi_demo.txt";

// Write initial content
io.writeFile(path, "Line 1\n");

// Append more lines
io.appendFile(path, "Line 2\n");
io.appendFile(path, "Line 3\n");

// Check existence and read
if (io.exists(path)) {
    var content = io.readFile(path);
    print(content);
    // Line 1
    // Line 2
    // Line 3
}

// Overwrite
io.writeFile(path, "Fresh start\n");
print(io.readFile(path));  // "Fresh start\n"
```

## Error Handling

Use `try/catch` to handle file errors gracefully:

```izilang
import "io";

try {
    var data = io.readFile("/nonexistent/path.txt");
} catch (e) {
    print("Could not read file:", e);
}
```

## Platform Notes

- `fileExists()` / `exists()` use POSIX `<sys/stat.h>` and may behave differently on Windows.
- File paths use the native OS path separator.

## See Also

- [Standard Library Index](README.md)
- [env module](env.md) — Read environment variables (e.g. for home directory)

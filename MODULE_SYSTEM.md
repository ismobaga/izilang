# IziLang Module System

## Overview

IziLang supports a comprehensive module system with `export` and `import` statements, allowing code organization across multiple files with relative imports and circular dependency detection.

## Features

### Relative Imports (v0.2+)

Import modules using relative paths:

```izilang
// Same directory
import "./config";

// Subdirectory
import "./lib/math";

// Parent directory
import "../shared_utils";
```

**Path Resolution:**
- Imports are resolved relative to the **importing file's directory**
- `.izi` extension is automatically added if not present
- Supports nested directory structures
- Module paths are canonicalized to prevent duplicate imports

### Export Declarations

Use `export` to make functions and variables available to other modules:

```izilang
export fn add(a, b) {
    return a + b;
}

export fn subtract(a, b) {
    return a - b;
}

export var PI = 3.14159;
export var E = 2.71828;
```

**Note**: Function exports are partially implemented. Variables work correctly.

### Import Statements

#### Simple Import
Import all exports from a module (they become globally available):

```izilang
import "math.iz";

print(add(5, 3));
print(PI);
```

#### Relative Import
Import using relative paths:

```izilang
import "./lib/math";

print(add(5, 3));
```

#### Named Imports
Import specific exports from a module:

```izilang
import { add, PI } from "math.iz";

print(add(5, 3));
print(PI);
```

#### Wildcard Import (Syntax Supported)
The syntax for wildcard imports is supported but namespace objects are not yet fully implemented:

```izilang
import * as math from "math.iz";
```

**Note**: Currently, all imports make exports globally available rather than creating separate namespaces.

## Module Resolution

### File-Based Modules
- **Relative imports** (starting with `./` or `../`) are resolved relative to the importing file's directory
- **Non-relative imports** are resolved relative to the current working directory
- Module names without `.izi` or `.iz` extension automatically get `.izi` appended
- Example: `import "./math"` resolves to `./math.izi` relative to the current file

### Native Modules
IziLang includes built-in native modules that are always available:
- `math`, `string`, `array`, `io`, `assert`, `log`, `path`
- See [STANDARD_LIBRARY.md](STANDARD_LIBRARY.md) for details

## Caching and Deduplication

- Modules are cached after the first import using **canonical paths**
- Re-importing the same module multiple times will not reload or re-execute it
- Same module imported via different relative paths (e.g., `./lib/a` and `../lib/a`) is correctly deduplicated

## Circular Import Detection

The module system detects circular dependencies and reports them with a clear error message:

```
Error: Circular import detected: /path/to/a.izi -> /path/to/b.izi -> /path/to/a.izi
```

This prevents infinite recursion and helps identify problematic dependencies.

## Examples

### Example 1: Simple Multi-File Project

**Project structure:**
```
my_app/
  main.izi
  config.izi
```

**config.izi**:
```izilang
var APP_NAME = "My App";
var VERSION = "1.0.0";
```

**main.izi**:
```izilang
import "./config";

print(APP_NAME);  // Prints: My App
print(VERSION);   // Prints: 1.0.0
```

### Example 2: Nested Modules

**Project structure:**
```
my_app/
  main.izi
  lib/
    math.izi
    utils.izi
```

**lib/math.izi**:
```izilang
var PI = 3.14159;
```

**lib/utils.izi**:
```izilang
import "./math";  // Relative to lib/ directory

var CIRCLE_AREA = PI * 10 * 10;
```

**main.izi**:
```izilang
import "./lib/utils";

print(CIRCLE_AREA);  // Uses PI from math.izi
```

### Example 3: Creating a Module

**utils.iz**:
```izilang
export fn square(n) {
    return n * n;
}

export fn cube(n) {
    return n * n * n;
}

export var MAX_SIZE = 100;
```

### Using a Module

**main.iz**:
```izilang
import { square, cube, MAX_SIZE } from "utils.iz";

print("Square of 5:", square(5));
print("Cube of 3:", cube(3));
print("Max size:", MAX_SIZE);
```

Or with simple import:

```izilang
import "utils.iz";

print("Square of 5:", square(5));
print("Cube of 3:", cube(3));
print("Max size:", MAX_SIZE);
```

## Complete Example

See [examples/multi_file_project/](examples/multi_file_project/) for a complete working example demonstrating:
- Relative imports
- Nested directory structures
- Module variables
- Project organization

## Limitations

1. **No namespace isolation**: Currently all exports become global after import. Named imports accept any identifiers but don't restrict access to non-imported names.

2. **Function exports**: While function export syntax is supported, calling exported functions may have issues. Variable exports work correctly.

3. **Wildcard imports**: While `import * as name from "module"` syntax is parsed and dot notation (e.g., `obj.property`) is supported for maps, wildcard imports do not create namespace objects. Therefore, you cannot use `math.add()` after `import * as math from "module"`. The parser will accept the syntax but runtime will fail because no namespace object is created. This requires tracking module exports separately and creating a map object, which is planned for a future enhancement.

4. **No re-exports**: You cannot re-export symbols from other modules yet.

5. **Export placement**: `export` must directly precede a function or variable declaration. You cannot export existing names or export multiple items in one statement.

## Future Enhancements

- Namespace objects for wildcard imports
- Module-level scope isolation
- Default exports
- Re-export syntax
- Complete function export support
- Private/public symbol visibility

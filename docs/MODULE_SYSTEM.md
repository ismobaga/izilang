# IziLang Module System

## Overview

IziLang supports a comprehensive module system with `export` and `import` statements, allowing code organization across multiple files with relative imports, namespace isolation, and circular dependency detection.

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

#### Default Exports

Export a default value using `export default`:

```izilang
// Export a default function
export default fn greet(name) {
    return "Hello " + name;
}

// Export a default expression
export default 42;

// Export a default variable
export default var CONFIG = { debug: true };
```

### Import Statements

#### Simple Import
Import all exports from a module (they become globally available):

```izilang
import "./math";

print(add(5, 3));
print(PI);
```

#### Named Imports
Import specific exports from a module:

```izilang
import { add, PI } from "./math";

print(add(5, 3));
print(PI);
```

#### Named Import with Alias
Import an export under a different local name:

```izilang
import { add as sum, PI as pi } from "./math";
import { default as greet } from "./greeter";

print(sum(5, 3));
print(greet("World"));
```

#### Wildcard Import (Namespace Object)
Import all exports into a namespace object:

```izilang
import * as math from "./math";
var result = math.add(5, 3);
var pi_val = math.PI;
```

Wildcard imports create a proper namespace object. Only **exported** names are accessible through the namespace.

### Re-Export Syntax

Re-export symbols from another module:

```izilang
// Re-export all exports from a module
export * from "./utils";

// Re-export specific named exports
export { add, subtract } from "./arithmetic";
```

This is useful for creating "barrel" index files:

```izilang
// lib/index.izi
export * from "./math";
export * from "./string";
export { format } from "./io";
```

## Module Scope Isolation

When using **wildcard** (`import * as name`) or **named** (`import { a, b }`) imports from file modules, the module runs in an **isolated scope**:

- Variables and functions **without** `export` are private to the module
- Only `export`-ed names are accessible through the namespace object or named bindings
- Prevents accidental name collisions between modules

```izilang
// config.izi
var SECRET_KEY = "abc123";       // private — not exported
export var APP_NAME = "MyApp";   // public — exported
export var VERSION = "1.0.0";    // public — exported
```

```izilang
// main.izi
import * as config from "./config";
print(config.APP_NAME);   // "MyApp" — OK
print(config.VERSION);    // "1.0.0" — OK
// config.SECRET_KEY is not accessible — isolation working
```

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

## Export Validation

When using named imports (`import { a, b } from "./module"`), the runtime validates that each requested name was actually exported by the module. Importing a non-exported name throws an error:

```
Error: Module './utils' does not export 'notExported'
```

## Examples

### Example 1: Simple Multi-File Project

**lib/math.izi**:
```izilang
export var PI = 3.14159;
export fn area(r) { return PI * r * r; }
var internalHelper = 42;   // private
```

**main.izi**:
```izilang
import * as math from "./lib/math";

print(math.PI);          // 3.14159
print(math.area(5));     // 78.53975
```

### Example 2: Named Imports

**utils.izi**:
```izilang
export fn square(n) { return n * n; }
export fn cube(n)   { return n * n * n; }
export var MAX_SIZE = 100;
```

**main.izi**:
```izilang
import { square, MAX_SIZE } from "./utils";

print(square(5));     // 25
print(MAX_SIZE);      // 100
```

### Example 3: Default Export

**greeter.izi**:
```izilang
export default fn(name) { return "Hello, " + name + "!"; }
```

**main.izi**:
```izilang
import { default as greet } from "./greeter";
print(greet("World"));   // Hello, World!
```

### Example 4: Re-Export Barrel

**lib/index.izi**:
```izilang
export * from "./math";
export { format } from "./io";
```

**main.izi**:
```izilang
import * as lib from "./lib/index";
var result = lib.square(4);
```

### Example 5: Nested Module Project

See [examples/multi_file_project/](examples/multi_file_project/) for a complete working example demonstrating:
- Relative imports
- Nested directory structures
- Module variables
- Project organization

## Limitations

1. **Simple imports (`import "module"`)**: Still run in the global scope for backward compatibility. All variables in the module become globally accessible. Use wildcard or named imports for proper namespace isolation.

2. **Bytecode compiler**: The bytecode VM (`--compile` mode) does not yet implement isolated module scope for re-exports.

3. **Export placement**: `export` must directly precede a function, variable declaration, or `default` keyword. You cannot export existing names without re-export syntax.

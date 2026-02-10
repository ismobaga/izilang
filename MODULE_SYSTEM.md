# IziLang Module System

## Overview

IziLang now supports a module system with `export` and `import` statements, allowing code organization across multiple files.

## Features

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

### Import Statements

#### Simple Import
Import all exports from a module (they become globally available):

```izilang
import "math.iz";

print(add(5, 3));
print(PI);
```

#### Named Imports
Import specific exports from a module:

```izilang
import { add, PI } from "math.iz";

print(add(5, 3));
print(PI);
```

#### Wildcard Import (Syntax Supported)
The syntax for wildcard imports is supported but namespace objects are not yet implemented:

```izilang
import * as math from "math.iz";
```

**Note**: Currently, all imports make exports globally available rather than creating separate namespaces.

## Module Resolution

- Modules are resolved relative to the current working directory
- Module names without `.iz` extension automatically get `.iz` appended
- Example: `import "math"` resolves to `math.iz`

## Caching

Modules are cached after the first import. Re-importing the same module multiple times in a file or across files will not reload or re-execute the module.

## Examples

### Creating a Module

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

## Limitations

1. **No namespace isolation**: Currently all exports become global after import. Named imports accept any identifiers but don't restrict access to non-imported names.

2. **Wildcard imports**: While `import * as name from "module"` syntax is parsed, namespace objects are not yet created. A future enhancement will add proper namespace support with member access (e.g., `math.add()`).

3. **Circular dependencies**: While module caching prevents infinite loops, circular dependencies may cause issues if modules depend on each other's exports during initialization.

4. **No re-exports**: You cannot re-export symbols from other modules yet.

5. **Export placement**: `export` must directly precede a function or variable declaration. You cannot export existing names or export multiple items in one statement.

## Future Enhancements

- Namespace objects for wildcard imports
- Module-level scope isolation
- Default exports
- Re-export syntax
- Circular dependency detection and handling
- Path resolution (relative and absolute paths)

# Multi-File Project Example

This example demonstrates IziLang's package and module system with support for:
- Relative imports using `./` and `../`
- Nested directory structures
- Module deduplication
- Circular import detection

## Project Structure

```
multi_file_project/
├── main.izi              # Entry point
├── config.izi            # Shared configuration
└── utils/
    ├── math_constants.izi  # Math utilities
    └── strings.izi         # String utilities
```

## Import Syntax

### Relative Imports (Same Directory)
```izi
import "./config";
```

### Relative Imports (Subdirectory)
```izi
import "./utils/math_constants";
```

### Relative Imports (Parent Directory)
```izi
import "../shared_module";
```

## Running the Example

From this directory:
```bash
izi main.izi
```

Or from the repository root:
```bash
izi examples/multi_file_project/main.izi
```

## Features Demonstrated

1. **Relative Path Resolution**: Imports are resolved relative to the importing file's directory
2. **Nested Modules**: Modules can import other modules, forming a dependency tree
3. **Module Deduplication**: Each module is loaded only once, even if imported multiple times
4. **Entry-Point Resolution**: The interpreter tracks which file is currently being executed

## Limitations

- **Function Exports**: The `export` keyword for functions is not yet fully implemented
- **Named Imports**: Currently, all symbols from an imported module are added to the global scope
- **Module Scope**: Imported modules execute in the global scope rather than their own module scope

## Future Enhancements

- Proper module scoping
- Named and wildcard imports
- Export statement support
- Private/public symbol visibility

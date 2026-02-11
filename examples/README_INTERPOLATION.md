# String Interpolation in IziLang

This example demonstrates the string interpolation feature in IziLang, which allows you to embed expressions directly within string literals using the `${}` syntax.

## Syntax

```javascript
var name = "Alice";
var message = "Hello, ${name}!";  // "Hello, Alice!"
```

## Features

- **Variable interpolation**: Embed variable values directly in strings
- **Expression evaluation**: Any expression can be used inside `${}`
- **Automatic type conversion**: Numbers, booleans, and other types are automatically converted to strings
- **Multiple interpolations**: Use multiple `${}` expressions in a single string
- **Nested expressions**: Complex expressions with operators are fully supported

## Examples

See `string_interpolation.iz` for a comprehensive set of examples including:
- Basic variable interpolation
- Number and boolean interpolation
- Expression evaluation (arithmetic, function calls)
- Multiple interpolations in one string
- Complex nested expressions

## Running the Examples

```bash
./izi examples/string_interpolation.iz
```

## Implementation Details

Internally, string interpolation is compiled to string concatenation operations with automatic type conversion using the `str()` function:

```javascript
"Hello, ${name}!"  →  "Hello, " + str(name) + "!"
```

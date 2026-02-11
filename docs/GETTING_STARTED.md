# Getting Started with IziLang

Welcome to IziLang! This guide will help you get started with installing, running, and using IziLang.

## Installation

### Building from Source

IziLang uses Premake5 for build configuration. Follow these steps to build from source:

#### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- Premake5 (included in the repository)
- Make (Linux/macOS) or MSBuild (Windows)

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/ismobaga/izilang.git
cd izilang

# Generate build files
./premake5 gmake2  # Use 'premake5 vs2022' on Windows

# Build debug version
make config=debug

# Build release version
make config=release

# The binary will be in bin/Debug/izi/ or bin/Release/izi/
```

### Installation

After building, you can create a symlink to use `izi` from anywhere:

```bash
# Linux/macOS
sudo ln -s $(pwd)/bin/Release/izi/izi /usr/local/bin/izi

# Or add to your PATH
export PATH="$PATH:$(pwd)/bin/Release/izi"
```

## Your First IziLang Program

Create a file called `hello.iz`:

```izilang
print("Hello, World!");
```

Run it with:

```bash
izi run hello.iz
# Or simply:
izi hello.iz
```

## Language Basics

### Variables

```izilang
var name = "Alice";
var age = 30;
var isStudent = false;
```

### Functions

```izilang
fn greet(name) {
    return "Hello, " + name + "!";
}

print(greet("Bob"));  // Output: Hello, Bob!
```

### Control Flow

```izilang
// If statements
var score = 85;
if (score >= 90) {
    print("Grade: A");
} else if (score >= 80) {
    print("Grade: B");
} else {
    print("Grade: C");
}

// While loops
var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}

// For loops
for (var j = 0; j < 5; j = j + 1) {
    print(j);
}
```

### Arrays

```izilang
var fruits = ["apple", "banana", "cherry"];
print(fruits[0]);  // Output: apple

// Array methods
push(fruits, "date");
var last = pop(fruits);
```

### Maps (Objects)

```izilang
var person = {
    name: "Alice",
    age: 30,
    city: "New York"
};

print(person["name"]);  // Output: Alice
print(person.age);       // Output: 30
```

### Pattern Matching

```izilang
fn describe(value) {
    match (value) {
        case 0 => print("Zero"),
        case 1 => print("One"),
        case _ => print("Other")
    }
}
```

### Exception Handling

```izilang
try {
    var result = riskyOperation();
    print(result);
} catch (e) {
    print("Error:", e);
} finally {
    print("Cleanup");
}
```

## CLI Commands

IziLang provides several commands for different workflows:

### `izi run <file>`

Execute a source file:

```bash
izi run script.iz
```

### `izi build <file>`

Check syntax and compile without executing:

```bash
izi build app.iz
```

### `izi check <file>`

Quick syntax check (parsing only):

```bash
izi check script.iz
```

### `izi test [pattern]`

Run test files:

```bash
# Run all tests
izi test

# Run tests matching a pattern
izi test collections
```

### `izi repl`

Start an interactive REPL:

```bash
izi repl
```

REPL commands:
- `:help` - Show available commands
- `:exit` - Exit the REPL
- `:reset` - Reset the environment
- `:debug` - Toggle debug mode

### Options

- `--vm` - Use bytecode VM (default: tree-walker interpreter)
- `--interp` - Use tree-walker interpreter (default)
- `--debug` - Enable debug/verbose output
- `--help` - Show help message
- `--version` - Show version information

## Examples

### Factorial

```izilang
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print(factorial(5));  // Output: 120
```

### FizzBuzz

```izilang
for (var i = 1; i <= 100; i = i + 1) {
    if (i % 15 == 0) {
        print("FizzBuzz");
    } else if (i % 3 == 0) {
        print("Fizz");
    } else if (i % 5 == 0) {
        print("Buzz");
    } else {
        print(i);
    }
}
```

### Working with Collections

```izilang
var numbers = [1, 2, 3, 4, 5];

// Filter even numbers
var evens = [];
for (var i = 0; i < len(numbers); i = i + 1) {
    if (numbers[i] % 2 == 0) {
        push(evens, numbers[i]);
    }
}

print(evens);  // Output: [2, 4]
```

## Next Steps

- Check out the [CLI Reference](CLI_REFERENCE.md) for detailed command documentation
- Read about [Editor Setup](EDITOR_SETUP.md) to get syntax highlighting and IDE support
- Explore example programs in the `examples/` directory
- Learn about [Error Handling](ERRORS_TROUBLESHOOTING.md) and common issues
- Visit the [Language Reference](LANGUAGE_REFERENCE.md) for comprehensive language documentation

## Getting Help

If you encounter issues:

1. Check the [Troubleshooting Guide](ERRORS_TROUBLESHOOTING.md)
2. Look at the [examples directory](../examples/)
3. Read the [documentation](.)
4. Open an issue on GitHub

## Contributing

We welcome contributions! See [CONTRIBUTING.md](../CONTRIBUTING.md) for guidelines.

---

**IziLang** - A modern, expressive programming language

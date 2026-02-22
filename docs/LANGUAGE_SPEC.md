# IziLang Language Specification

**Version**: 0.4  
**Status**: Active

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Lexical Structure](#2-lexical-structure)
3. [Grammar (EBNF)](#3-grammar-ebnf)
4. [Type System](#4-type-system)
5. [Expressions](#5-expressions)
6. [Statements](#6-statements)
7. [Functions](#7-functions)
8. [Classes](#8-classes)
9. [Modules](#9-modules)
10. [Error Handling](#10-error-handling)
11. [Pattern Matching](#11-pattern-matching)
12. [Semantics](#12-semantics)

---

## 1. Introduction

IziLang is a dynamically-typed, expression-oriented scripting language with optional gradual type annotations. It supports imperative, functional, and object-oriented programming styles.

### Design Goals

- **Readable syntax** - Code should be easy to read and write
- **Safe by default** - Clear errors with helpful messages
- **Gradual typing** - Add types when you want them
- **Multiple execution models** - Tree-walking interpreter and bytecode VM

---

## 2. Lexical Structure

### 2.1 Source Files

IziLang source files use the `.iz` or `.izi` extension and are encoded in UTF-8.

### 2.2 Comments

```
// Single-line comment

/* Multi-line
   comment */
```

### 2.3 Keywords

The following identifiers are reserved:

```
and       break     catch     class     continue  else
extends   false     finally   fn        for       if
import    match     nil       not       or        return
super     this      throw     true      try       var
while     async     await     macro     export    from
as        case
```

### 2.4 Operators

```
+   -   *   /   %       // Arithmetic
==  !=  <   >   <=  >=  // Comparison
and or  not             // Logical
=                       // Assignment
+=  -=  *=  /=          // Compound assignment
!                       // Logical not (alias for `not`)
->                      // Arrow (function type annotations)
...                     // Spread
```

### 2.5 Delimiters

```
(  )   // Parentheses
{  }   // Braces
[  ]   // Brackets
,      // Comma
;      // Semicolon (statement terminator)
.      // Member access
:      // Type annotation
```

### 2.6 Literals

#### Number Literals

Numbers are 64-bit floating-point (IEEE 754 double):

```
42
3.14
0.001
1000000
```

#### String Literals

Strings are delimited by double quotes:

```
"Hello, World!"
"Line 1\nLine 2"
"Tab\there"
```

**Escape sequences:**

| Sequence | Meaning       |
|----------|---------------|
| `\\`     | Backslash     |
| `\"`     | Double quote  |
| `\n`     | Newline       |
| `\t`     | Tab           |
| `\r`     | Carriage return |

**String interpolation** uses `${expression}`:

```
"Hello, ${name}!"
"Result: ${x + y}"
"${a} * ${b} = ${a * b}"
```

#### Boolean Literals

```
true
false
```

#### Nil Literal

```
nil
```

### 2.7 Identifiers

Identifiers start with a letter or underscore, followed by letters, digits, or underscores:

```
EBNF:
identifier = ( letter | "_" ) { letter | digit | "_" } ;
letter     = "a"-"z" | "A"-"Z" ;
digit      = "0"-"9" ;
```

---

## 3. Grammar (EBNF)

The following grammar uses Extended Backus-Naur Form (EBNF). Terminals are in quotes, non-terminals in angle brackets.

```ebnf
(* Program *)
program     = { declaration } EOF ;

(* Declarations *)
declaration = class_decl
            | fn_decl
            | var_decl
            | import_decl
            | statement ;

class_decl  = "class" IDENT [ "extends" IDENT ] "{" { method_decl } "}" ;
method_decl = "fn" IDENT "(" [ param_list ] ")" [ ":" type ] block ;

fn_decl     = [ "async" ] "fn" IDENT "(" [ param_list ] ")" [ ":" type ] block ;

var_decl    = "var" IDENT [ ":" type ] [ "=" expression ] ";" ;

import_decl = "import" import_spec ";"
            | "import" STRING ";" ;
import_spec = "{" IDENT { "," IDENT } "}" "from" STRING
            | "*" "as" IDENT "from" STRING ;

(* Statements *)
statement   = expr_stmt
            | if_stmt
            | while_stmt
            | for_stmt
            | return_stmt
            | break_stmt
            | continue_stmt
            | throw_stmt
            | try_stmt
            | block ;

expr_stmt   = expression ";" ;
block       = "{" { declaration } "}" ;

if_stmt     = "if" "(" expression ")" block
              [ "else" ( if_stmt | block ) ] ;

while_stmt  = "while" "(" expression ")" block ;

for_stmt    = "for" "(" ( var_decl | expr_stmt | ";" )
              [ expression ] ";"
              [ expression ] ")" block ;

return_stmt = "return" [ expression ] ";" ;
break_stmt  = "break" ";" ;
continue_stmt = "continue" ";" ;
throw_stmt  = "throw" expression ";" ;

try_stmt    = "try" block
              [ "catch" "(" IDENT ")" block ]
              [ "finally" block ] ;

(* Expressions — by precedence, lowest first *)
expression  = assignment ;

assignment  = [ call "." ] IDENT ( "=" | "+=" | "-=" | "*=" | "/=" ) assignment
            | conditional ;

conditional = null_coalesce [ "?" expression ":" conditional ] ;

null_coalesce = logic_or { "??" logic_or } ;

match_expr  = "match" expression "{" { match_arm } "}" | logic_or ;
match_arm   = pattern [ "if" expression ] "=>" expression "," ;
pattern     = "_" | literal | IDENT ;

logic_or    = logic_and { "or" logic_and } ;
logic_and   = equality { "and" equality } ;
equality    = comparison { ( "!=" | "==" ) comparison } ;
comparison  = addition { ( ">" | ">=" | "<" | "<=" ) addition } ;
addition    = multiplication { ( "-" | "+" ) multiplication } ;
multiplication = unary { ( "/" | "*" | "%" ) unary } ;

unary       = ( "!" | "-" | "not" ) unary | await_expr ;
await_expr  = "await" call | call ;
call        = primary { "(" [ arg_list ] ")" | "[" expression "]" | "." IDENT } ;

primary     = NUMBER | STRING | "true" | "false" | "nil"
            | "this" | "super" "." IDENT
            | IDENT
            | "(" expression ")"
            | "[" [ spread_list ] "]"
            | "{" [ map_entries ] "}"
            | fn_expr
            | interpolated_string ;

fn_expr         = [ "async" ] "fn" "(" [ param_list ] ")" [ ":" type ] block ;
interpolated_string = '"' { char | "${" expression "}" } '"' ;

(* Helpers *)
param_list  = param { "," param } ;
param       = IDENT [ ":" type ] ;
arg_list    = ( expression | "..." expression ) { "," ( expression | "..." expression ) } ;
spread_list = ( expression | "..." expression ) { "," ( expression | "..." expression ) } ;
map_entries = map_entry { "," map_entry } ;
map_entry   = ( IDENT | STRING ) ":" expression
            | "..." expression ;

(* Types *)
type        = "Number" | "String" | "Bool" | "Nil" | "Any" | "Void"
            | "Array" [ "<" type ">" ]
            | "Map" [ "<" type "," type ">" ]
            | "Function" "(" [ type_list ] ")" "->" type
            | IDENT ;
type_list   = type { "," type } ;

(* Literals *)
literal     = NUMBER | STRING | "true" | "false" | "nil" ;
```

---

## 4. Type System

### 4.1 Primitive Types

| Type     | Description                    | Literals              |
|----------|--------------------------------|-----------------------|
| `Number` | 64-bit IEEE 754 floating point | `42`, `3.14`, `-1`   |
| `String` | Immutable UTF-8 string         | `"hello"`, `""`      |
| `Bool`   | Boolean true/false             | `true`, `false`      |
| `Nil`    | Absence of a value             | `nil`                |

### 4.2 Compound Types

| Type       | Description                         | Literals               |
|------------|-------------------------------------|------------------------|
| `Array`    | Ordered, mutable, heterogeneous     | `[1, "two", true]`    |
| `Map`      | Unordered key-value pairs           | `{a: 1, "b": 2}`      |
| `Function` | First-class function value          | `fn(x) { return x; }` |
| Class instance | User-defined object             | `MyClass(args)`        |

### 4.3 Special Types

| Type   | Description                                  |
|--------|----------------------------------------------|
| `Any`  | Compatible with all types (default)          |
| `Void` | Represents no return value                   |

### 4.4 Gradual Typing

IziLang supports **optional type annotations**. Code without annotations is fully dynamic and behaves as if all values have type `Any`.

```izilang
// Dynamic (no annotations) — always valid
var x = 42;
fn add(a, b) { return a + b; }

// Annotated — checked by `izi check`
var x: Number = 42;
fn add(a: Number, b: Number): Number { return a + b; }
```

Type annotations are **checked statically** by `izi check` and ignored at runtime.

### 4.5 Type Compatibility Rules

1. `Any` is compatible with all types
2. All types are compatible with `Any`
3. Exact type match required for annotated code (no implicit conversions)
4. Function parameter and return types are checked independently

### 4.6 Truthiness

In boolean context (conditions, logical operators), values are treated as:

| Value                 | Truthiness |
|-----------------------|------------|
| `false`               | falsy      |
| `nil`                 | falsy      |
| Everything else       | truthy     |

This includes `0`, `""`, and `[]` — they are **truthy** in IziLang (unlike JavaScript or Python).

### 4.7 Type Coercions

IziLang has **no implicit type coercions**. Operations on incompatible types throw a runtime error:

```izilang
1 + "2"     // Error: Cannot add number and string
true + 1    // Error: Operands must be numbers
```

The `+` operator is overloaded only for:
- `Number + Number` → addition
- `String + String` → concatenation

---

## 5. Expressions

### 5.1 Arithmetic Operators

| Operator | Description            | Operand Types   |
|----------|------------------------|-----------------|
| `+`      | Addition / concatenation | Number, String |
| `-`      | Subtraction            | Number          |
| `*`      | Multiplication         | Number          |
| `/`      | Division               | Number          |
| `%`      | Modulo (remainder)     | Number          |
| `-` (unary) | Negation            | Number          |

### 5.2 Comparison Operators

All comparison operators return `Bool`:

| Operator | Description              | Operand Types    |
|----------|--------------------------|------------------|
| `==`     | Equality                 | Any              |
| `!=`     | Inequality               | Any              |
| `<`      | Less than                | Number           |
| `>`      | Greater than             | Number           |
| `<=`     | Less than or equal       | Number           |
| `>=`     | Greater than or equal    | Number           |

**Equality** uses value equality for primitives and reference equality for objects:

```izilang
1 == 1        // true
"hi" == "hi"  // true
nil == nil    // true
nil == false  // false
```

### 5.3 Logical Operators

| Operator | Description           | Short-circuit |
|----------|-----------------------|---------------|
| `and`    | Logical AND           | Yes (left)    |
| `or`     | Logical OR            | Yes (left)    |
| `not`    | Logical NOT           | No            |
| `!`      | Alias for `not`       | No            |

`and` and `or` return one of their operands (not necessarily a `Bool`):

```izilang
nil or "default"     // "default"
"value" and "other"  // "other"
false or 42          // 42
```

### 5.4 Assignment Operators

```izilang
x = 10        // simple assignment
x += 5        // x = x + 5
x -= 3        // x = x - 3
x *= 2        // x = x * 2
x /= 4        // x = x / 4
```

### 5.5 Operator Precedence

From highest to lowest:

| Precedence | Operators                  |
|------------|---------------------------|
| 1 (highest)| `()`, `[]`, `.` (call/access) |
| 2          | `!`, `-` (unary), `not`   |
| 3          | `*`, `/`, `%`             |
| 4          | `+`, `-`                  |
| 5          | `<`, `>`, `<=`, `>=`      |
| 6          | `==`, `!=`                |
| 7          | `and`                     |
| 8          | `or`                      |
| 9 (lowest) | `=`, `+=`, `-=`, `*=`, `/=` |

### 5.6 Array Expressions

```izilang
[]              // empty array
[1, 2, 3]       // array literal
[...a, ...b]    // spread: concatenate arrays
```

### 5.7 Map Expressions

```izilang
{}                      // empty map
{a: 1, b: 2}            // map literal (identifier keys)
{"key": value}          // string key
{...defaults, x: 10}    // spread: merge maps
```

### 5.8 Function Expressions

```izilang
fn(x) { return x * 2; }               // anonymous function
fn(x: Number): Number { return x; }   // typed anonymous function
async fn(url) { return await fetch(url); }  // async anonymous function
```

---

## 6. Statements

### 6.1 Variable Declaration

```izilang
var name = "Alice";           // dynamic
var age: Number = 30;         // typed
var count: Number;            // typed, initialized to nil
```

Variables declared with `var` are **mutable** and scoped to the enclosing block.

### 6.2 If Statement

```izilang
if (condition) {
    // then
} else if (other) {
    // else-if
} else {
    // else
}
```

### 6.3 While Loop

```izilang
while (condition) {
    // body
}
```

### 6.4 For Loop

```izilang
for (var i = 0; i < 10; i = i + 1) {
    // body
}

// All parts are optional:
for (;;) { break; }  // infinite loop
```

### 6.5 Break and Continue

```izilang
while (true) {
    if (done) { break; }     // exit loop
    if (skip) { continue; }  // next iteration
}
```

### 6.6 Return Statement

```izilang
fn example() {
    return 42;       // return with value
}

fn nothing() {
    return;          // return nil
}
```

A function without an explicit `return` returns `nil`.

### 6.7 Throw Statement

```izilang
throw "Something went wrong!";
throw {code: 404, message: "Not found"};
```

Any value can be thrown.

### 6.8 Try-Catch-Finally

```izilang
try {
    riskyOperation();
} catch (error) {
    print("Error:", error);
} finally {
    cleanup();  // always executes
}
```

- `catch` clause is optional
- `finally` clause is optional
- At least one of `catch` or `finally` must be present
- `finally` always executes, even if an exception is re-thrown

---

## 7. Functions

### 7.1 Function Declaration

```izilang
fn name(param1, param2) {
    return result;
}

// With type annotations
fn add(a: Number, b: Number): Number {
    return a + b;
}

// Async function
async fn fetchData(url: String): Any {
    return await http.get(url);
}
```

### 7.2 Function Call

```izilang
name(arg1, arg2)
obj.method(arg)
```

### 7.3 First-Class Functions

Functions are values that can be stored, passed, and returned:

```izilang
var double = fn(x) { return x * 2; };
print(double(5));  // 10

fn apply(f, x) { return f(x); }
print(apply(double, 5));  // 10
```

### 7.4 Closures

Functions capture their enclosing scope:

```izilang
fn makeCounter() {
    var count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
}

var counter = makeCounter();
print(counter());  // 1
print(counter());  // 2
print(counter());  // 3
```

### 7.5 Variadic Functions

The standard library supports variadic functions via native C++. User-defined variadic functions are not yet supported.

### 7.6 Async Functions

```izilang
async fn fetchUser(id) {
    var response = await http.get("https://api.example.com/users/" + id);
    return response.body;
}
```

`await` suspends the async function until the awaited value resolves.

---

## 8. Classes

### 8.1 Class Declaration

```izilang
class ClassName {
    fn init(param1, param2) {
        this.field1 = param1;
        this.field2 = param2;
    }

    fn method() {
        return this.field1;
    }
}
```

The `init` method (or `constructor`) is called automatically when creating an instance.

### 8.2 Instantiation

```izilang
var obj = ClassName(arg1, arg2);
```

There is no `new` keyword — call the class name as a function.

### 8.3 Member Access

```izilang
obj.field         // read field
obj.field = val   // write field
obj.method()      // call method
```

### 8.4 The `this` Keyword

Inside methods, `this` refers to the current instance:

```izilang
class Circle {
    fn init(radius) {
        this.radius = radius;
    }

    fn area() {
        return 3.14159 * this.radius * this.radius;
    }
}
```

### 8.5 Inheritance

```izilang
class Animal {
    fn init(name) {
        this.name = name;
    }

    fn speak() {
        print(this.name, "makes a sound");
    }
}

class Dog extends Animal {
    fn init(name) {
        super.init(name);  // call parent method
        this.species = "Canine";
    }

    fn speak() {
        print(this.name, "barks!");  // override
    }
}
```

- Single inheritance only
- `super.method(args)` calls the parent class method
- Child classes inherit all parent methods
- Override parent methods by defining a method with the same name

---

## 9. Modules

### 9.1 Import Statements

```izilang
// Import module (creates a module object)
import "math";
math.sqrt(16)

// Import specific names
import { sqrt, pi } from "math";
sqrt(16)

// Import with alias
import * as m from "math";
m.sqrt(16)

// Import standard library with qualified name
import * as json from "std.json";
json.parse("{}")
```

### 9.2 Standard Library Modules

| Module       | Import Name   | Description                        |
|--------------|---------------|------------------------------------|
| Math         | `"math"`      | Mathematical functions             |
| String utils | `"string"`    | String manipulation                |
| Array utils  | `"array"`     | Array higher-order functions       |
| File I/O     | `"io"`        | File read/write                    |
| Assertions   | `"assert"`    | Runtime assertion helpers          |
| JSON         | `"std.json"`  | JSON parsing and serialization     |
| Time         | `"std.time"`  | Time and sleep operations          |
| Regex        | `"std.regex"` | Regular expression operations      |
| HTTP client  | `"std.http"`  | HTTP GET/POST requests             |
| Environment  | `"std.env"`   | Environment variable access        |
| IPC          | `"std.ipc"`   | Named-pipe inter-process comms     |
| UI           | `"ui"`        | Graphical window (requires raylib) |

See [STANDARD_LIBRARY.md](STANDARD_LIBRARY.md) for detailed API reference.

---

## 10. Error Handling

### 10.1 Runtime Errors

Runtime errors terminate execution with a message that includes:
- Error type and message
- Source file name
- Line number and column
- Visual code snippet with a `^` pointer

Example output:

```
In file 'script.iz':
Runtime Error at line 3, column 10:
  3 | var z = x + y;
    |          ^
Cannot add number and string. Operands must be two numbers or two strings.
```

### 10.2 Exception Handling

```izilang
try {
    var result = parseInt("not a number");
} catch (e) {
    print("Caught:", e);  // e is the thrown value
} finally {
    print("Cleanup done");
}
```

Any value can be caught — the `e` variable holds whatever was thrown.

### 10.3 Throwing Exceptions

```izilang
throw "error message";
throw {type: "ValidationError", message: "Invalid input"};
```

### 10.4 Error Propagation

Uncaught exceptions propagate up the call stack. If they reach the top level without being caught, the program terminates with an error message.

---

## 11. Pattern Matching

### 11.1 Match Expression

```izilang
var result = match value {
    pattern1 => expr1,
    pattern2 => expr2,
    _ => default_expr
};
```

`match` is an **expression** that returns a value.

### 11.2 Patterns

| Pattern type      | Syntax              | Description                        |
|-------------------|---------------------|------------------------------------|
| Literal           | `42`, `"text"`, `true` | Matches exact value             |
| Wildcard          | `_`                 | Matches anything, binds nothing    |
| Variable          | `x`                 | Matches anything, binds to `x`     |
| Guard             | `x if condition`    | Matches if condition is truthy     |

### 11.3 Examples

```izilang
// Literal patterns
fn grade(score) {
    return match score {
        100 => "Perfect",
        x if x >= 90 => "A",
        x if x >= 80 => "B",
        x if x >= 70 => "C",
        _ => "F"
    };
}

// Boolean patterns
fn yesNo(b) {
    return match b {
        true => "yes",
        false => "no"
    };
}

// Nested match
fn classify(x, y) {
    return match x {
        0 => match y {
            0 => "origin",
            _ => "y-axis"
        },
        _ => match y {
            0 => "x-axis",
            _ => "other"
        }
    };
}
```

---

## 12. Semantics

### 12.1 Scoping Rules

IziLang uses **lexical (static) scoping**:

- Variables are visible from their declaration to the end of the enclosing block
- Inner scopes can access outer scope variables (closure)
- Variable shadowing: a declaration in an inner scope hides the outer declaration

```izilang
var x = 1;
{
    var x = 2;  // shadows outer x
    print(x);   // 2
}
print(x);       // 1 — outer x unchanged
```

### 12.2 Variable Lifetime

- Variables are created when their `var` declaration is executed
- Variables are alive until the enclosing block exits
- Closures extend the lifetime of captured variables beyond the block

### 12.3 Evaluation Order

- Expressions are evaluated **left to right**
- Function arguments are evaluated before the call
- Short-circuit evaluation applies to `and` and `or`

### 12.4 Function Call Semantics

- Arguments are passed **by value** for primitives (Number, String, Bool, Nil)
- Arrays and Maps are passed **by reference** (mutations inside the function are visible outside)
- Functions are closures — they capture their defining environment

```izilang
fn mutateArray(arr) {
    push(arr, 99);
}

var numbers = [1, 2, 3];
mutateArray(numbers);
print(numbers);  // [1, 2, 3, 99] — mutated
```

### 12.5 String Immutability

Strings are **immutable**. String operations always return new strings:

```izilang
var s = "hello";
var upper = toUpper(s);
print(s);     // "hello" — unchanged
print(upper); // "HELLO"
```

### 12.6 Equality Semantics

- Primitives (`Number`, `String`, `Bool`, `Nil`): **value equality**
- Arrays and Maps: **reference equality** (two separate literals are not equal)
- Class instances: **reference equality**

```izilang
1 == 1          // true
"a" == "a"      // true
nil == nil      // true
[1] == [1]      // false — different objects
```

### 12.7 Nil and Uninitialized Values

- Uninitialized variables (declared without a value) hold `nil`
- Accessing a non-existent map key returns `nil`
- Accessing a non-existent class field returns `nil`
- Arithmetic on `nil` raises a runtime error

### 12.8 Class Instantiation Semantics

1. A new empty map-like object is created
2. The `init` (or `constructor`) method is called with the given arguments
3. Fields set with `this.field = value` are stored on the object
4. The object is returned to the caller

### 12.9 Inheritance and Method Resolution

Method resolution follows the class hierarchy:

1. Look for method on the instance's class
2. If not found, look in the parent class
3. Continue up the chain until found or error

```izilang
class A {
    fn greet() { print("Hello from A"); }
}
class B extends A {}

var b = B();
b.greet();  // "Hello from A" — inherited
```

### 12.10 Module Semantics

- Modules are loaded once and cached
- All names imported from a module share the same module instance
- Module imports at the top level are executed at program start

---

## Appendix A: Reserved Identifiers

The following names are used by the standard library and should be avoided as user identifiers: `print`, `clock`, `len`, `push`, `pop`, `keys`, `values`, `hasKey`, `toString`, `toNumber`, `typeOf`, `sqrt`, `abs`, `floor`, `ceil`, `round`.

## Appendix B: Version History

| Version | Key Changes |
|---------|-------------|
| 0.1     | Core language: variables, functions, control flow, arrays, maps |
| 0.2     | REPL, CLI, error messages, module system |
| 0.3     | Classes, gradual typing, pattern matching, GC, bytecode VM |
| 0.4     | Async/await, macros, string interpolation, spread/destructuring, rich stdlib |

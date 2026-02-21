# Learn IziLang in Y Minutes

> A fast-paced tour of the IziLang language, inspired by the "Learn X in Y Minutes" format.  
> Each section builds on the previous one — complete beginners should start at the top.

---

## Table of Contents

1. [Hello, World!](#1-hello-world)
2. [Variables and Types](#2-variables-and-types)
3. [Arithmetic and Operators](#3-arithmetic-and-operators)
4. [Strings](#4-strings)
5. [Control Flow](#5-control-flow)
6. [Functions](#6-functions)
7. [Arrays](#7-arrays)
8. [Maps](#8-maps)
9. [Closures](#9-closures)
10. [Classes and Objects](#10-classes-and-objects)
11. [Pattern Matching](#11-pattern-matching)
12. [Error Handling](#12-error-handling)
13. [Modules](#13-modules)
14. [Exercises](#14-exercises)

---

## 1. Hello, World!

Save this in `hello.iz` and run `izi hello.iz`:

```izilang
print("Hello, World!");
```

The `print` function is always available — no imports needed.

---

## 2. Variables and Types

Declare variables with `var`:

```izilang
var name = "Alice";      // String
var age = 30;            // Number (always 64-bit float)
var isStudent = false;   // Bool
var score = 9.5;         // Number (decimals just work)
var nothing = nil;       // Nil — absence of value
```

Variables are **mutable** — you can reassign them:

```izilang
var x = 1;
x = 2;
print(x);  // 2
```

You can optionally add **type annotations** — useful for documentation and checked by `izi check`:

```izilang
var count: Number = 0;
var greeting: String = "Hello";
var active: Bool = true;
```

Check the type of a value:

```izilang
print(typeOf(42));        // "number"
print(typeOf("hello"));   // "string"
print(typeOf(true));      // "bool"
print(typeOf(nil));       // "nil"
print(typeOf([1, 2]));    // "array"
print(typeOf({a: 1}));    // "map"
```

---

## 3. Arithmetic and Operators

```izilang
// Arithmetic
print(10 + 3);   // 13
print(10 - 3);   // 7
print(10 * 3);   // 30
print(10 / 3);   // 3.3333...
print(10 % 3);   // 1  (modulo/remainder)

// Comparison — return Bool
print(5 > 3);    // true
print(5 < 3);    // false
print(5 == 5);   // true
print(5 != 3);   // true
print(5 >= 5);   // true
print(5 <= 4);   // false

// Logical operators
print(true and false);  // false
print(true or false);   // true
print(not true);        // false

// Compound assignment
var x = 10;
x += 5;   // x = 15
x -= 3;   // x = 12
x *= 2;   // x = 24
x /= 4;   // x = 6
```

**Important**: IziLang has **no implicit type conversion**. You cannot add a number and a string:

```izilang
1 + "2"   // Runtime Error!
```

To convert types explicitly:

```izilang
toString(42)          // "42"
toNumber("3.14")      // 3.14
```

---

## 4. Strings

Strings are created with double quotes:

```izilang
var s = "Hello, World!";
print(len(s));  // 13
```

**String concatenation** with `+`:

```izilang
var first = "Hello";
var second = "World";
print(first + ", " + second + "!");  // Hello, World!
```

**String interpolation** with `${...}`:

```izilang
var name = "Alice";
var age = 30;
print("My name is ${name} and I am ${age} years old.");
// My name is Alice and I am 30 years old.

// Expressions work too
var x = 5;
var y = 3;
print("${x} + ${y} = ${x + y}");  // 5 + 3 = 8
```

**Common string functions** (from the `string` module):

```izilang
import "string";

var s = "  Hello, World!  ";
print(string.trim(s));              // "Hello, World!"
print(string.toUpper("hello"));     // "HELLO"
print(string.toLower("HELLO"));     // "hello"
print(string.substring(s, 2, 5));   // "Hello"
print(string.replace(s, "World", "IziLang"));  // "  Hello, IziLang!  "

var parts = string.split("a,b,c", ",");  // ["a", "b", "c"]
print(string.join(parts, "-"));          // "a-b-c"

print(string.indexOf("hello", "ll"));    // 2
print(string.startsWith("hello", "he")); // true
print(string.endsWith("hello", "lo"));   // true
```

---

## 5. Control Flow

### If/Else

```izilang
var score = 85;

if (score >= 90) {
    print("Grade: A");
} else if (score >= 80) {
    print("Grade: B");
} else if (score >= 70) {
    print("Grade: C");
} else {
    print("Grade: F");
}
// Output: Grade: B
```

### While Loop

```izilang
var i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
// Output: 0 1 2 3 4
```

### For Loop

```izilang
for (var i = 0; i < 5; i = i + 1) {
    print(i);
}
// Output: 0 1 2 3 4
```

### Break and Continue

```izilang
for (var i = 0; i < 10; i = i + 1) {
    if (i == 3) { continue; }  // skip 3
    if (i == 7) { break; }     // stop at 7
    print(i);
}
// Output: 0 1 2 4 5 6
```

---

## 6. Functions

Declare functions with `fn`:

```izilang
fn greet(name) {
    return "Hello, " + name + "!";
}

print(greet("Alice"));  // Hello, Alice!
```

Functions without a `return` statement return `nil`:

```izilang
fn sayHi() {
    print("Hi!");
}
var result = sayHi();  // prints "Hi!"
print(result);         // nil
```

### Default Parameter Workaround

IziLang doesn't have default parameters, but you can check for `nil`:

```izilang
fn greet(name) {
    if (name == nil) { name = "stranger"; }
    return "Hello, " + name + "!";
}
print(greet(nil));    // Hello, stranger!
print(greet("Bob"));  // Hello, Bob!
```

### Recursive Functions

```izilang
fn factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}

print(factorial(5));  // 120
print(factorial(10)); // 3628800
```

### Functions as Values

Functions are first-class values:

```izilang
fn double(x) { return x * 2; }
fn square(x) { return x * x; }

fn apply(f, x) {
    return f(x);
}

print(apply(double, 5));  // 10
print(apply(square, 5));  // 25
```

### Anonymous Functions (Lambdas)

```izilang
var add = fn(a, b) { return a + b; };
print(add(3, 4));  // 7

// Pass directly to another function
fn apply(f, x) { return f(x); }
print(apply(fn(x) { return x * 3; }, 5));  // 15
```

---

## 7. Arrays

Create arrays with `[]`:

```izilang
var fruits = ["apple", "banana", "cherry"];
var numbers = [1, 2, 3, 4, 5];
var mixed = [1, "two", true, nil];  // mixed types allowed
var empty = [];
```

**Indexing** (zero-based):

```izilang
print(fruits[0]);   // "apple"
print(fruits[1]);   // "banana"
print(fruits[2]);   // "cherry"

fruits[1] = "blueberry";
print(fruits);  // [apple, blueberry, cherry]
```

**Array operations**:

```izilang
var arr = [1, 2, 3];

push(arr, 4);      // add to end → [1, 2, 3, 4]
var last = pop(arr);  // remove from end → returns 4, arr = [1, 2, 3]
print(len(arr));   // 3
```

**Array spread** — combine arrays without mutation:

```izilang
var a = [1, 2, 3];
var b = [4, 5, 6];
var combined = [...a, ...b];   // [1, 2, 3, 4, 5, 6]
var extended = [0, ...a, 99];  // [0, 1, 2, 3, 99]
```

**Array destructuring**:

```izilang
var coords = [10, 20, 30];
var [x, y, z] = coords;
print(x);  // 10
print(y);  // 20
print(z);  // 30
```

**Higher-order array functions** (from the `array` module):

```izilang
import { map, filter, reduce, sort } from "array";

var numbers = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

// map — transform each element
var doubled = map(numbers, fn(x) { return x * 2; });
print(doubled);  // [2, 4, 6, 8, 10, 12, 14, 16, 18, 20]

// filter — keep matching elements
var evens = filter(numbers, fn(x) { return x % 2 == 0; });
print(evens);  // [2, 4, 6, 8, 10]

// reduce — accumulate to a single value
var total = reduce(numbers, fn(acc, x) { return acc + x; }, 0);
print(total);  // 55

// sort
var unsorted = [3, 1, 4, 1, 5, 9, 2, 6];
var sorted = sort(unsorted);
print(sorted);  // [1, 1, 2, 3, 4, 5, 6, 9]
```

---

## 8. Maps

Maps are key-value collections (like objects in JavaScript or dicts in Python):

```izilang
var person = {
    name: "Alice",
    age: 30,
    city: "New York"
};
```

**Access values** with `.` or `[]`:

```izilang
print(person.name);       // Alice
print(person["age"]);     // 30
print(person["missing"]); // nil — missing keys return nil
```

**Set values**:

```izilang
person.email = "alice@example.com";
person["age"] = 31;
```

**Check keys**:

```izilang
print(hasKey(person, "name"));  // true
print(hasKey(person, "phone")); // false
print(keys(person));            // [name, age, city, email]
print(values(person));          // [Alice, 31, New York, alice@example.com]
```

**Map spread** — merge maps:

```izilang
var defaults = {timeout: 5000, retries: 3, verbose: false};
var overrides = {retries: 5, verbose: true};
var config = {...defaults, ...overrides};
print(config);  // {timeout: 5000, retries: 5, verbose: true}
```

**Map destructuring**:

```izilang
var point = {x: 10, y: 20, z: 30};
var {x, y, z} = point;
print(x);  // 10
print(y);  // 20
```

---

## 9. Closures

Functions can capture variables from their enclosing scope:

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

// Each counter has its own state
var c1 = makeCounter();
var c2 = makeCounter();
print(c1());  // 1
print(c1());  // 2
print(c2());  // 1 — independent from c1
```

Closures are perfect for creating "factories" and encapsulating state:

```izilang
fn makeAdder(n) {
    return fn(x) { return x + n; };
}

var add5 = makeAdder(5);
var add10 = makeAdder(10);

print(add5(3));   // 8
print(add10(3));  // 13
```

---

## 10. Classes and Objects

Define a class with `class`:

```izilang
class Rectangle {
    fn init(width, height) {
        this.width = width;
        this.height = height;
    }

    fn area() {
        return this.width * this.height;
    }

    fn perimeter() {
        return 2 * (this.width + this.height);
    }

    fn describe() {
        print("Rectangle ${this.width}x${this.height}");
    }
}

var rect = Rectangle(4, 6);
rect.describe();           // Rectangle 4x6
print(rect.area());        // 24
print(rect.perimeter());   // 20
```

### Inheritance

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
    fn init(name, breed) {
        super.init(name);  // call parent init
        this.breed = breed;
    }

    fn speak() {
        print(this.name, "barks!");  // override parent method
    }

    fn info() {
        print(this.name, "is a", this.breed);
    }
}

var dog = Dog("Buddy", "Labrador");
dog.speak();  // Buddy barks!
dog.info();   // Buddy is a Labrador
```

---

## 11. Pattern Matching

`match` is an expression — it returns a value:

```izilang
var day = 3;
var name = match day {
    1 => "Monday",
    2 => "Tuesday",
    3 => "Wednesday",
    4 => "Thursday",
    5 => "Friday",
    _ => "Weekend"
};
print(name);  // Wednesday
```

**Guards** add conditions to patterns:

```izilang
fn classify(n) {
    return match n {
        0 => "zero",
        x if x > 0 => "positive",
        x if x < 0 => "negative",
        _ => "other"
    };
}

print(classify(5));   // positive
print(classify(-3));  // negative
print(classify(0));   // zero
```

**FizzBuzz** with pattern matching:

```izilang
fn fizzBuzz(n) {
    return match [n % 3 == 0, n % 5 == 0] {
        [true, true]   => "FizzBuzz",
        [true, false]  => "Fizz",
        [false, true]  => "Buzz",
        _              => toString(n)
    };
}

for (var i = 1; i <= 15; i = i + 1) {
    print(fizzBuzz(i));
}
```

---

## 12. Error Handling

### Basic Try/Catch

```izilang
try {
    var result = 10 / 0;  // This works (returns Infinity in IEEE 754)
    throw "Something went wrong!";
} catch (error) {
    print("Caught:", error);  // Caught: Something went wrong!
}
```

### Try/Catch/Finally

```izilang
fn readAndProcess(filename) {
    var file = nil;
    try {
        file = io.readFile(filename);
        return processData(file);
    } catch (e) {
        print("Error processing file:", e);
        return nil;
    } finally {
        print("Done with", filename);  // always runs
    }
}
```

### Throwing Custom Errors

```izilang
fn divide(a, b) {
    if (b == 0) {
        throw {type: "DivisionByZero", message: "Cannot divide by zero"};
    }
    return a / b;
}

try {
    print(divide(10, 2));   // 5
    print(divide(10, 0));   // throws
} catch (e) {
    print("Error:", e.type, "-", e.message);
    // Error: DivisionByZero - Cannot divide by zero
}
```

---

## 13. Modules

IziLang has a built-in module system:

```izilang
// Import a module — use it as an object
import "math";
print(math.pi);        // 3.14159...
print(math.sqrt(16));  // 4

// Import specific names
import { sqrt, sin, cos, pi } from "math";
print(sqrt(25));       // 5
print(sin(pi / 2));    // 1

// Import with an alias
import * as m from "math";
print(m.pow(2, 8));    // 256
```

### Common Modules

```izilang
import "string";
import "array";
import "io";
import * as json from "std.json";
import * as time from "std.time";
import * as regex from "std.regex";
import * as env from "std.env";
```

See the [Standard Library Reference](STANDARD_LIBRARY.md) for all available functions.

---

## 14. Exercises

Practice what you've learned! Solutions are in the `examples/` directory.

### Exercise 1 — Basics

Write a function `celsiusToFahrenheit(c)` that converts Celsius to Fahrenheit using the formula `F = (C × 9/5) + 32`.

```izilang
// Expected output:
print(celsiusToFahrenheit(0));    // 32
print(celsiusToFahrenheit(100));  // 212
print(celsiusToFahrenheit(37));   // 98.6
```

<details>
<summary>Solution</summary>

```izilang
fn celsiusToFahrenheit(c) {
    return (c * 9 / 5) + 32;
}
```

</details>

---

### Exercise 2 — Loops and Accumulation

Write a function `sumRange(start, end)` that returns the sum of all integers from `start` to `end` (inclusive).

```izilang
// Expected output:
print(sumRange(1, 10));   // 55
print(sumRange(5, 5));    // 5
print(sumRange(1, 100));  // 5050
```

<details>
<summary>Solution</summary>

```izilang
fn sumRange(start, end) {
    var total = 0;
    for (var i = start; i <= end; i = i + 1) {
        total = total + i;
    }
    return total;
}
```

</details>

---

### Exercise 3 — Arrays

Write a function `countOccurrences(arr, target)` that returns how many times `target` appears in `arr`.

```izilang
// Expected output:
print(countOccurrences([1, 2, 3, 2, 1, 2], 2));     // 3
print(countOccurrences(["a", "b", "a"], "a"));       // 2
print(countOccurrences([1, 2, 3], 4));               // 0
```

<details>
<summary>Solution</summary>

```izilang
fn countOccurrences(arr, target) {
    var count = 0;
    for (var i = 0; i < len(arr); i = i + 1) {
        if (arr[i] == target) {
            count = count + 1;
        }
    }
    return count;
}
```

</details>

---

### Exercise 4 — Maps

Write a function `wordCount(text)` that returns a map of each word to its frequency.

```izilang
import "string";

// Expected output:
var counts = wordCount("hello world hello");
print(counts["hello"]);  // 2
print(counts["world"]);  // 1
```

<details>
<summary>Solution</summary>

```izilang
import "string";

fn wordCount(text) {
    var words = string.split(text, " ");
    var counts = {};
    for (var i = 0; i < len(words); i = i + 1) {
        var word = words[i];
        if (hasKey(counts, word)) {
            counts[word] = counts[word] + 1;
        } else {
            counts[word] = 1;
        }
    }
    return counts;
}
```

</details>

---

### Exercise 5 — Closures

Write a function `memoize(f)` that wraps a function with a cache so that repeated calls with the same argument return the cached result.

```izilang
fn slowSquare(n) {
    // Imagine this is slow
    return n * n;
}

var fastSquare = memoize(slowSquare);
print(fastSquare(4));  // 16 (computed)
print(fastSquare(4));  // 16 (from cache)
print(fastSquare(5));  // 25 (computed)
```

<details>
<summary>Solution</summary>

```izilang
fn memoize(f) {
    var cache = {};
    return fn(x) {
        var key = toString(x);
        if (hasKey(cache, key)) {
            return cache[key];
        }
        var result = f(x);
        cache[key] = result;
        return result;
    };
}
```

</details>

---

### Exercise 6 — Classes

Write a `Stack` class with `push(value)`, `pop()`, `peek()`, `isEmpty()`, and `size()` methods.

```izilang
var s = Stack();
print(s.isEmpty());  // true
s.push(1);
s.push(2);
s.push(3);
print(s.size());     // 3
print(s.peek());     // 3
print(s.pop());      // 3
print(s.size());     // 2
```

<details>
<summary>Solution</summary>

```izilang
class Stack {
    fn init() {
        this.items = [];
    }

    fn push(value) {
        push(this.items, value);
    }

    fn pop() {
        return pop(this.items);
    }

    fn peek() {
        return this.items[len(this.items) - 1];
    }

    fn isEmpty() {
        return len(this.items) == 0;
    }

    fn size() {
        return len(this.items);
    }
}
```

</details>

---

### Exercise 7 — Pattern Matching

Write a function `httpStatus(code)` that returns a human-readable description of an HTTP status code.

```izilang
// Expected output:
print(httpStatus(200));  // "OK"
print(httpStatus(404));  // "Not Found"
print(httpStatus(500));  // "Internal Server Error"
print(httpStatus(301));  // "3xx Redirect"
print(httpStatus(999));  // "Unknown"
```

<details>
<summary>Solution</summary>

```izilang
fn httpStatus(code) {
    return match code {
        200 => "OK",
        201 => "Created",
        204 => "No Content",
        301 => "Moved Permanently",
        302 => "Found",
        400 => "Bad Request",
        401 => "Unauthorized",
        403 => "Forbidden",
        404 => "Not Found",
        500 => "Internal Server Error",
        503 => "Service Unavailable",
        x if x >= 300 and x < 400 => "3xx Redirect",
        x if x >= 400 and x < 500 => "4xx Client Error",
        x if x >= 500 and x < 600 => "5xx Server Error",
        _ => "Unknown"
    };
}
```

</details>

---

### Exercise 8 — Error Handling

Write a function `safeDiv(a, b)` that returns `{ok: true, value: result}` on success or `{ok: false, error: message}` on failure (division by zero). Do not use try/catch.

```izilang
// Expected output:
var r1 = safeDiv(10, 2);
print(r1.ok);     // true
print(r1.value);  // 5

var r2 = safeDiv(10, 0);
print(r2.ok);     // false
print(r2.error);  // "Division by zero"
```

<details>
<summary>Solution</summary>

```izilang
fn safeDiv(a, b) {
    if (b == 0) {
        return {ok: false, error: "Division by zero"};
    }
    return {ok: true, value: a / b};
}
```

</details>

---

## What's Next?

- **[Standard Library Reference](STANDARD_LIBRARY.md)** — All built-in modules
- **[Language Specification](LANGUAGE_SPEC.md)** — Formal grammar and semantics
- **[Cookbook](COOKBOOK.md)** — Practical patterns and real-world examples
- **[Examples directory](../examples/)** — Runnable example programs
- **[CLI Reference](CLI_REFERENCE.md)** — All `izi` commands

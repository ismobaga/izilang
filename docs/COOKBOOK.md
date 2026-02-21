# IziLang Cookbook

> Practical patterns, best practices, and real-world examples.  
> All examples are runnable with `izi run <file>.iz`.

---

## Table of Contents

1. [Common Patterns](#1-common-patterns)
   - [Null Safety](#11-null-safety)
   - [Default Values](#12-default-values)
   - [Guard Clauses](#13-guard-clauses)
   - [Accumulator Pattern](#14-accumulator-pattern)
   - [Builder Pattern](#15-builder-pattern)
2. [Working with Collections](#2-working-with-collections)
   - [Filtering and Transforming Arrays](#21-filtering-and-transforming-arrays)
   - [Grouping Data](#22-grouping-data)
   - [Sorting with a Comparator](#23-sorting-with-a-comparator)
   - [Deduplication](#24-deduplication)
3. [String Processing](#3-string-processing)
   - [Parsing CSV Lines](#31-parsing-csv-lines)
   - [Template Rendering](#32-template-rendering)
   - [String Padding](#33-string-padding)
4. [Functional Programming Patterns](#4-functional-programming-patterns)
   - [Compose and Pipe](#41-compose-and-pipe)
   - [Memoization](#42-memoization)
   - [Currying](#43-currying)
5. [Object-Oriented Patterns](#5-object-oriented-patterns)
   - [Observer Pattern](#51-observer-pattern)
   - [Strategy Pattern](#52-strategy-pattern)
   - [Linked List](#53-linked-list)
6. [Error Handling Patterns](#6-error-handling-patterns)
   - [Result Type](#61-result-type)
   - [Retry Logic](#62-retry-logic)
   - [Validation](#63-validation)
7. [Working with Files and I/O](#7-working-with-files-and-io)
   - [Read and Parse a Config File](#71-read-and-parse-a-config-file)
   - [Write a Log File](#72-write-a-log-file)
   - [Batch File Processing](#73-batch-file-processing)
8. [Working with JSON](#8-working-with-json)
   - [Parse and Query JSON](#81-parse-and-query-json)
   - [Build and Serialize JSON](#82-build-and-serialize-json)
9. [Mathematical Computations](#9-mathematical-computations)
   - [Statistics](#91-statistics)
   - [Prime Numbers](#92-prime-numbers)
   - [Matrix Operations](#93-matrix-operations)
10. [Real-World Examples](#10-real-world-examples)
    - [To-Do List Manager](#101-to-do-list-manager)
    - [Simple Calculator](#102-simple-calculator)
    - [Word Frequency Counter](#103-word-frequency-counter)
    - [Mini Test Runner](#104-mini-test-runner)

---

## 1. Common Patterns

### 1.1 Null Safety

IziLang returns `nil` for missing map keys and uninitialized variables. Always guard against `nil` before using a value:

```izilang
fn getOrDefault(map, key, defaultValue) {
    var value = map[key];
    if (value == nil) {
        return defaultValue;
    }
    return value;
}

var config = {port: 8080};
print(getOrDefault(config, "port", 3000));    // 8080
print(getOrDefault(config, "host", "localhost")); // "localhost"
```

Use a helper to safely chain map lookups:

```izilang
fn safeGet(obj, key) {
    if (obj == nil) { return nil; }
    return obj[key];
}

var response = {data: {user: {name: "Alice"}}};
var name = safeGet(safeGet(safeGet(response, "data"), "user"), "name");
print(name);  // "Alice"
```

---

### 1.2 Default Values

Pattern for handling optional parameters:

```izilang
fn createUser(name, options) {
    if (options == nil) { options = {}; }
    var role = options["role"];
    var active = options["active"];
    if (role == nil) { role = "user"; }
    if (active == nil) { active = true; }

    return {
        name: name,
        role: role,
        active: active
    };
}

var alice = createUser("Alice", {role: "admin"});
print(alice.name);    // "Alice"
print(alice.role);    // "admin"
print(alice.active);  // true

var bob = createUser("Bob", nil);
print(bob.role);    // "user"
print(bob.active);  // true
```

---

### 1.3 Guard Clauses

Use early returns to reduce nesting and improve readability:

**Before (deeply nested):**

```izilang
fn processOrder(order) {
    if (order != nil) {
        if (order.items != nil) {
            if (len(order.items) > 0) {
                // process order
                return "processed";
            }
        }
    }
    return "failed";
}
```

**After (guard clauses):**

```izilang
fn processOrder(order) {
    if (order == nil) { return "failed"; }
    if (order.items == nil) { return "failed"; }
    if (len(order.items) == 0) { return "failed"; }

    // process order
    return "processed";
}
```

---

### 1.4 Accumulator Pattern

Build up a result by accumulating values in a loop:

```izilang
fn buildReport(items) {
    var lines = [];
    var total = 0;

    for (var i = 0; i < len(items); i = i + 1) {
        var item = items[i];
        var line = "${item.name}: $${item.price}";
        push(lines, line);
        total = total + item.price;
    }

    push(lines, "---");
    push(lines, "Total: $${total}");
    return lines;
}

import "string";
var items = [
    {name: "Apple", price: 1.5},
    {name: "Bread", price: 2.99},
    {name: "Milk", price: 3.25}
];
var report = buildReport(items);
print(string.join(report, "\n"));
// Apple: $1.5
// Bread: $2.99
// Milk: $3.25
// ---
// Total: $7.74
```

---

### 1.5 Builder Pattern

Use a class or closures to build complex objects step by step:

```izilang
class QueryBuilder {
    fn init(table) {
        this.table = table;
        this.conditions = [];
        this.limitVal = nil;
        this.orderCol = nil;
    }

    fn where(condition) {
        push(this.conditions, condition);
        return this;  // enable chaining
    }

    fn limit(n) {
        this.limitVal = n;
        return this;
    }

    fn orderBy(col) {
        this.orderCol = col;
        return this;
    }

    fn build() {
        import "string";
        var query = "SELECT * FROM " + this.table;
        if (len(this.conditions) > 0) {
            query = query + " WHERE " + string.join(this.conditions, " AND ");
        }
        if (this.orderCol != nil) {
            query = query + " ORDER BY " + this.orderCol;
        }
        if (this.limitVal != nil) {
            query = query + " LIMIT " + toString(this.limitVal);
        }
        return query;
    }
}

var query = QueryBuilder("users")
    .where("age > 18")
    .where("active = true")
    .orderBy("name")
    .limit(10)
    .build();

print(query);
// SELECT * FROM users WHERE age > 18 AND active = true ORDER BY name LIMIT 10
```

---

## 2. Working with Collections

### 2.1 Filtering and Transforming Arrays

```izilang
import { map, filter, reduce } from "array";

var products = [
    {name: "Apple",  price: 1.50, category: "fruit"},
    {name: "Banana", price: 0.75, category: "fruit"},
    {name: "Bread",  price: 2.99, category: "bakery"},
    {name: "Milk",   price: 3.25, category: "dairy"},
    {name: "Cheese", price: 5.99, category: "dairy"}
];

// Get all fruits
var fruits = filter(products, fn(p) { return p.category == "fruit"; });
print(len(fruits));  // 2

// Get product names
var names = map(products, fn(p) { return p.name; });
print(names);  // [Apple, Banana, Bread, Milk, Cheese]

// Total price of dairy products
var dairy = filter(products, fn(p) { return p.category == "dairy"; });
var dairyTotal = reduce(dairy, fn(sum, p) { return sum + p.price; }, 0);
print(dairyTotal);  // 9.24

// Chain: get names of products under $2
var cheap = filter(products, fn(p) { return p.price < 2; });
var cheapNames = map(cheap, fn(p) { return p.name; });
print(cheapNames);  // [Apple, Banana]
```

---

### 2.2 Grouping Data

Group array items by a key:

```izilang
fn groupBy(arr, keyFn) {
    var groups = {};
    for (var i = 0; i < len(arr); i = i + 1) {
        var item = arr[i];
        var key = keyFn(item);
        if (!hasKey(groups, key)) {
            groups[key] = [];
        }
        push(groups[key], item);
    }
    return groups;
}

var items = [
    {name: "Apple",  category: "fruit"},
    {name: "Banana", category: "fruit"},
    {name: "Bread",  category: "bakery"},
    {name: "Milk",   category: "dairy"},
    {name: "Cheese", category: "dairy"}
];

var grouped = groupBy(items, fn(item) { return item.category; });
print(len(grouped["fruit"]));   // 2
print(len(grouped["dairy"]));   // 2
print(len(grouped["bakery"]));  // 1
```

---

### 2.3 Sorting with a Comparator

Use bubble sort with a custom comparator:

```izilang
fn sortBy(arr, compareFn) {
    // Bubble sort (simple, not efficient for large arrays)
    var result = [...arr];
    var n = len(result);
    for (var i = 0; i < n - 1; i = i + 1) {
        for (var j = 0; j < n - i - 1; j = j + 1) {
            if (compareFn(result[j], result[j + 1]) > 0) {
                var temp = result[j];
                result[j] = result[j + 1];
                result[j + 1] = temp;
            }
        }
    }
    return result;
}

var people = [
    {name: "Charlie", age: 25},
    {name: "Alice",   age: 30},
    {name: "Bob",     age: 22}
];

// Sort by age
var byAge = sortBy(people, fn(a, b) { return a.age - b.age; });
print(byAge[0].name);  // Bob (youngest)
print(byAge[2].name);  // Alice (oldest)
```

---

### 2.4 Deduplication

Remove duplicate values from an array:

```izilang
fn unique(arr) {
    var seen = {};
    var result = [];
    for (var i = 0; i < len(arr); i = i + 1) {
        var key = toString(arr[i]);
        if (!hasKey(seen, key)) {
            seen[key] = true;
            push(result, arr[i]);
        }
    }
    return result;
}

var nums = [1, 2, 3, 2, 1, 4, 3, 5];
print(unique(nums));  // [1, 2, 3, 4, 5]

var tags = ["js", "python", "js", "rust", "python"];
print(unique(tags));  // [js, python, rust]
```

---

## 3. String Processing

### 3.1 Parsing CSV Lines

```izilang
import "string";

fn parseCSVLine(line) {
    var fields = string.split(line, ",");
    var result = [];
    for (var i = 0; i < len(fields); i = i + 1) {
        push(result, string.trim(fields[i]));
    }
    return result;
}

fn parseCSV(content) {
    var lines = string.split(content, "\n");
    var rows = [];
    for (var i = 0; i < len(lines); i = i + 1) {
        var line = string.trim(lines[i]);
        if (len(line) > 0) {
            push(rows, parseCSVLine(line));
        }
    }
    return rows;
}

var csv = "Alice, 30, Engineer\nBob, 25, Designer\nCharlie, 35, Manager";
var rows = parseCSV(csv);
for (var i = 0; i < len(rows); i = i + 1) {
    print("Name: " + rows[i][0] + ", Age: " + rows[i][1]);
}
// Name: Alice, Age: 30
// Name: Bob, Age: 25
// Name: Charlie, Age: 35
```

---

### 3.2 Template Rendering

Simple string template substitution:

```izilang
import "string";

fn renderTemplate(template, vars) {
    var result = template;
    var varKeys = keys(vars);
    for (var i = 0; i < len(varKeys); i = i + 1) {
        var key = varKeys[i];
        result = string.replace(result, "{{" + key + "}}", toString(vars[key]));
    }
    return result;
}

var tmpl = "Hello, {{name}}! You have {{count}} new messages.";
var output = renderTemplate(tmpl, {name: "Alice", count: 5});
print(output);  // Hello, Alice! You have 5 new messages.
```

---

### 3.3 String Padding

Pad strings for tabular output:

```izilang
fn padLeft(s, width) {
    var str = toString(s);
    while (len(str) < width) {
        str = " " + str;
    }
    return str;
}

fn padRight(s, width) {
    var str = toString(s);
    while (len(str) < width) {
        str = str + " ";
    }
    return str;
}

fn printTable(headers, rows) {
    var widths = [];
    for (var i = 0; i < len(headers); i = i + 1) {
        var w = len(headers[i]);
        for (var j = 0; j < len(rows); j = j + 1) {
            var cellLen = len(toString(rows[j][i]));
            if (cellLen > w) { w = cellLen; }
        }
        push(widths, w + 2);
    }

    import "string";
    var headerRow = "";
    for (var i = 0; i < len(headers); i = i + 1) {
        headerRow = headerRow + padRight(headers[i], widths[i]);
    }
    print(headerRow);

    for (var j = 0; j < len(rows); j = j + 1) {
        var row = "";
        for (var i = 0; i < len(rows[j]); i = i + 1) {
            row = row + padRight(toString(rows[j][i]), widths[i]);
        }
        print(row);
    }
}

printTable(
    ["Name", "Age", "City"],
    [
        ["Alice", 30, "New York"],
        ["Bob", 25, "London"],
        ["Charlie", 35, "Tokyo"]
    ]
);
// Name      Age   City
// Alice     30    New York
// Bob       25    London
// Charlie   35    Tokyo
```

---

## 4. Functional Programming Patterns

### 4.1 Compose and Pipe

Compose functions right-to-left (compose) or left-to-right (pipe):

```izilang
// compose(f, g)(x) = f(g(x))
fn compose(f, g) {
    return fn(x) { return f(g(x)); };
}

// pipe(f, g)(x) = g(f(x))
fn pipe(f, g) {
    return fn(x) { return g(f(x)); };
}

import "string";

fn double(x) { return x * 2; }
fn addTen(x) { return x + 10; }
fn stringify(x) { return "Result: " + toString(x); }

var transform = pipe(pipe(double, addTen), stringify);
print(transform(5));  // Result: 20  (5*2=10, 10+10=20)
```

---

### 4.2 Memoization

Cache function results to avoid redundant computation:

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

// Fibonacci without memoization is exponential
fn fibSlow(n) {
    if (n <= 1) { return n; }
    return fibSlow(n - 1) + fibSlow(n - 2);
}

// With memoization — fast!
var fib;  // forward reference
fib = memoize(fn(n) {
    if (n <= 1) { return n; }
    return fib(n - 1) + fib(n - 2);
});

print(fib(10));  // 55
print(fib(30));  // 832040
print(fib(40));  // 102334155
```

---

### 4.3 Currying

Transform a multi-argument function into a chain of single-argument functions:

```izilang
fn curry2(f) {
    return fn(a) {
        return fn(b) { return f(a, b); };
    };
}

fn add(a, b) { return a + b; }
fn multiply(a, b) { return a * b; }

var curriedAdd = curry2(add);
var add5 = curriedAdd(5);

print(add5(3));   // 8
print(add5(10));  // 15

// Build reusable transformations
import { map } from "array";

var double = curry2(multiply)(2);
var triple = curry2(multiply)(3);

print(map([1, 2, 3, 4], double));  // [2, 4, 6, 8]
print(map([1, 2, 3, 4], triple));  // [3, 6, 9, 12]
```

---

## 5. Object-Oriented Patterns

### 5.1 Observer Pattern

Implement a simple event system:

```izilang
class EventEmitter {
    fn init() {
        this.listeners = {};
    }

    fn on(event, handler) {
        if (!hasKey(this.listeners, event)) {
            this.listeners[event] = [];
        }
        push(this.listeners[event], handler);
    }

    fn emit(event, data) {
        if (!hasKey(this.listeners, event)) { return; }
        var handlers = this.listeners[event];
        for (var i = 0; i < len(handlers); i = i + 1) {
            handlers[i](data);
        }
    }
}

var emitter = EventEmitter();

emitter.on("login", fn(user) {
    print("User logged in:", user.name);
});

emitter.on("login", fn(user) {
    print("Audit log: login by", user.name);
});

emitter.emit("login", {name: "Alice", time: "2026-01-01"});
// User logged in: Alice
// Audit log: login by Alice
```

---

### 5.2 Strategy Pattern

Choose an algorithm at runtime:

```izilang
class Sorter {
    fn init(strategy) {
        this.strategy = strategy;
    }

    fn sort(arr) {
        return this.strategy(arr);
    }
}

import { sort } from "array";

fn bubbleSort(arr) {
    var result = [...arr];
    var n = len(result);
    for (var i = 0; i < n; i = i + 1) {
        for (var j = 0; j < n - i - 1; j = j + 1) {
            if (result[j] > result[j + 1]) {
                var tmp = result[j];
                result[j] = result[j + 1];
                result[j + 1] = tmp;
            }
        }
    }
    return result;
}

fn reverseSort(arr) {
    var sorted = sort(arr);
    import { reverse } from "array";
    return reverse(sorted);
}

var data = [3, 1, 4, 1, 5, 9, 2, 6];

var ascSorter = Sorter(sort);
var descSorter = Sorter(reverseSort);

print(ascSorter.sort(data));   // [1, 1, 2, 3, 4, 5, 6, 9]
print(descSorter.sort(data));  // [9, 6, 5, 4, 3, 2, 1, 1]
```

---

### 5.3 Linked List

A classic data structure implemented with classes:

```izilang
class Node {
    fn init(value) {
        this.value = value;
        this.next = nil;
    }
}

class LinkedList {
    fn init() {
        this.head = nil;
        this.size = 0;
    }

    fn prepend(value) {
        var node = Node(value);
        node.next = this.head;
        this.head = node;
        this.size = this.size + 1;
    }

    fn append(value) {
        var node = Node(value);
        if (this.head == nil) {
            this.head = node;
        } else {
            var current = this.head;
            while (current.next != nil) {
                current = current.next;
            }
            current.next = node;
        }
        this.size = this.size + 1;
    }

    fn toArray() {
        var result = [];
        var current = this.head;
        while (current != nil) {
            push(result, current.value);
            current = current.next;
        }
        return result;
    }
}

var list = LinkedList();
list.append(1);
list.append(2);
list.append(3);
list.prepend(0);
print(list.toArray());  // [0, 1, 2, 3]
print(list.size);       // 4
```

---

## 6. Error Handling Patterns

### 6.1 Result Type

Return a result object instead of throwing exceptions:

```izilang
fn ok(value) {
    return {success: true, value: value};
}

fn err(message) {
    return {success: false, error: message};
}

fn parseInt(s) {
    var n = toNumber(s);
    if (n == nil) {
        return err("Not a valid number: " + s);
    }
    return ok(n);
}

fn safeDivide(a, b) {
    if (b == 0) {
        return err("Division by zero");
    }
    return ok(a / b);
}

var r = safeDivide(10, 2);
if (r.success) {
    print("Result:", r.value);  // Result: 5
} else {
    print("Error:", r.error);
}

var r2 = safeDivide(10, 0);
if (r2.success) {
    print("Result:", r2.value);
} else {
    print("Error:", r2.error);  // Error: Division by zero
}
```

---

### 6.2 Retry Logic

Retry an operation with exponential backoff:

```izilang
import * as time from "std.time";

fn retry(f, maxAttempts, delaySeconds) {
    var attempt = 0;
    while (attempt < maxAttempts) {
        attempt = attempt + 1;
        try {
            return f();
        } catch (e) {
            if (attempt >= maxAttempts) {
                throw e;
            }
            print("Attempt " + toString(attempt) + " failed, retrying in " + toString(delaySeconds) + "s...");
            time.sleep(delaySeconds);
            delaySeconds = delaySeconds * 2;  // exponential backoff
        }
    }
}

var callCount = 0;
fn unreliableOperation() {
    callCount = callCount + 1;
    if (callCount < 3) {
        throw "Temporary failure";
    }
    return "Success!";
}

var result = retry(unreliableOperation, 5, 0.1);
print(result);  // "Success!" (after 2 retries)
```

---

### 6.3 Validation

Build a reusable validation system:

```izilang
fn validate(data, rules) {
    var errors = [];

    var ruleKeys = keys(rules);
    for (var i = 0; i < len(ruleKeys); i = i + 1) {
        var field = ruleKeys[i];
        var rule = rules[field];
        var value = data[field];

        var result = rule(value);
        if (result != nil) {
            push(errors, field + ": " + result);
        }
    }

    return errors;
}

fn required(value) {
    if (value == nil or value == "") {
        return "is required";
    }
    return nil;
}

fn minLength(min) {
    return fn(value) {
        if (value == nil or len(value) < min) {
            return "must be at least " + toString(min) + " characters";
        }
        return nil;
    };
}

fn maxLength(max) {
    return fn(value) {
        if (value != nil and len(value) > max) {
            return "must be at most " + toString(max) + " characters";
        }
        return nil;
    };
}

var formData = {name: "Al", email: ""};
var rules = {
    name: minLength(3),
    email: required
};

var errors = validate(formData, rules);
if (len(errors) > 0) {
    print("Validation failed:");
    for (var i = 0; i < len(errors); i = i + 1) {
        print("  -", errors[i]);
    }
} else {
    print("Validation passed!");
}
// Validation failed:
//   - name: must be at least 3 characters
//   - email: is required
```

---

## 7. Working with Files and I/O

### 7.1 Read and Parse a Config File

```izilang
import "io";
import "string";
import * as json from "std.json";

fn readConfig(filename) {
    if (!io.exists(filename)) {
        return {};
    }
    var content = io.readFile(filename);
    return json.parse(content);
}

// Write a sample config
io.writeFile("/tmp/config.json", '{"port": 8080, "debug": true, "host": "localhost"}');

var config = readConfig("/tmp/config.json");
print("Host:", config.host);   // localhost
print("Port:", config.port);   // 8080
print("Debug:", config.debug); // true
```

---

### 7.2 Write a Log File

```izilang
import "io";
import * as time from "std.time";

fn createLogger(filename) {
    return fn(level, message) {
        var timestamp = time.format(time.now());
        var line = "[" + timestamp + "] [" + level + "] " + message + "\n";
        io.appendFile(filename, line);
    };
}

var log = createLogger("/tmp/app.log");

log("INFO", "Application started");
log("DEBUG", "Loading configuration");
log("WARN", "Config file not found, using defaults");
log("INFO", "Server listening on port 8080");
log("ERROR", "Database connection failed");

print(io.readFile("/tmp/app.log"));
```

---

### 7.3 Batch File Processing

```izilang
import "io";
import "string";
import { map } from "array";

fn processFiles(filenames, processFn) {
    var results = [];
    for (var i = 0; i < len(filenames); i = i + 1) {
        var filename = filenames[i];
        try {
            if (io.exists(filename)) {
                var content = io.readFile(filename);
                var result = processFn(content, filename);
                push(results, {file: filename, result: result, ok: true});
            } else {
                push(results, {file: filename, error: "File not found", ok: false});
            }
        } catch (e) {
            push(results, {file: filename, error: toString(e), ok: false});
        }
    }
    return results;
}

// Create test files
io.writeFile("/tmp/file1.txt", "Hello World");
io.writeFile("/tmp/file2.txt", "IziLang is great");

fn countWords(content, filename) {
    import "string";
    var words = string.split(string.trim(content), " ");
    return len(words);
}

var files = ["/tmp/file1.txt", "/tmp/file2.txt", "/tmp/missing.txt"];
var results = processFiles(files, countWords);

for (var i = 0; i < len(results); i = i + 1) {
    var r = results[i];
    if (r.ok) {
        print(r.file + ": " + toString(r.result) + " words");
    } else {
        print(r.file + ": ERROR - " + r.error);
    }
}
// /tmp/file1.txt: 2 words
// /tmp/file2.txt: 3 words
// /tmp/missing.txt: ERROR - File not found
```

---

## 8. Working with JSON

### 8.1 Parse and Query JSON

```izilang
import * as json from "std.json";

var jsonStr = '{
    "users": [
        {"name": "Alice", "age": 30, "active": true},
        {"name": "Bob",   "age": 25, "active": false},
        {"name": "Carol", "age": 35, "active": true}
    ]
}';

var data = json.parse(jsonStr);
var users = data.users;

print("Total users:", len(users));  // 3

// Filter active users
var active = [];
for (var i = 0; i < len(users); i = i + 1) {
    if (users[i].active) {
        push(active, users[i]);
    }
}
print("Active users:", len(active));  // 2

// Get all names
for (var i = 0; i < len(active); i = i + 1) {
    print("-", active[i].name);
}
```

---

### 8.2 Build and Serialize JSON

```izilang
import * as json from "std.json";
import "io";

fn buildUserRecord(name, email, age) {
    return {
        name: name,
        email: email,
        age: age,
        createdAt: "2026-01-01"
    };
}

var users = [
    buildUserRecord("Alice", "alice@example.com", 30),
    buildUserRecord("Bob",   "bob@example.com",   25)
];

var database = {
    version: 1,
    users: users
};

var jsonStr = json.stringify(database);
io.writeFile("/tmp/users.json", jsonStr);
print("Saved", len(users), "users to /tmp/users.json");

// Round-trip
var loaded = json.parse(io.readFile("/tmp/users.json"));
print("Loaded version:", loaded.version);         // 1
print("First user:", loaded.users[0].name);       // Alice
```

---

## 9. Mathematical Computations

### 9.1 Statistics

```izilang
import "math";

fn mean(arr) {
    var total = 0;
    for (var i = 0; i < len(arr); i = i + 1) {
        total = total + arr[i];
    }
    return total / len(arr);
}

fn variance(arr) {
    var m = mean(arr);
    var sumSq = 0;
    for (var i = 0; i < len(arr); i = i + 1) {
        var diff = arr[i] - m;
        sumSq = sumSq + diff * diff;
    }
    return sumSq / len(arr);
}

fn stddev(arr) {
    return math.sqrt(variance(arr));
}

fn median(arr) {
    import { sort } from "array";
    var sorted = sort(arr);
    var n = len(sorted);
    if (n % 2 == 0) {
        return (sorted[n / 2 - 1] + sorted[n / 2]) / 2;
    }
    return sorted[(n - 1) / 2];
}

var data = [2, 4, 4, 4, 5, 5, 7, 9];
print("Mean:   ", mean(data));    // 5
print("Median: ", median(data));  // 4.5
print("StdDev: ", stddev(data));  // 2
```

---

### 9.2 Prime Numbers

```izilang
fn isPrime(n) {
    if (n < 2) { return false; }
    if (n == 2) { return true; }
    if (n % 2 == 0) { return false; }

    import "math";
    var limit = math.floor(math.sqrt(n));
    for (var i = 3; i <= limit; i = i + 2) {
        if (n % i == 0) { return false; }
    }
    return true;
}

fn primesUpTo(limit) {
    var primes = [];
    for (var i = 2; i <= limit; i = i + 1) {
        if (isPrime(i)) {
            push(primes, i);
        }
    }
    return primes;
}

print(primesUpTo(50));
// [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47]
```

---

### 9.3 Matrix Operations

```izilang
fn createMatrix(rows, cols, fill) {
    var m = [];
    for (var i = 0; i < rows; i = i + 1) {
        var row = [];
        for (var j = 0; j < cols; j = j + 1) {
            push(row, fill);
        }
        push(m, row);
    }
    return m;
}

fn matrixMultiply(a, b) {
    var rowsA = len(a);
    var colsA = len(a[0]);
    var colsB = len(b[0]);
    var result = createMatrix(rowsA, colsB, 0);

    for (var i = 0; i < rowsA; i = i + 1) {
        for (var j = 0; j < colsB; j = j + 1) {
            for (var k = 0; k < colsA; k = k + 1) {
                result[i][j] = result[i][j] + a[i][k] * b[k][j];
            }
        }
    }
    return result;
}

var a = [[1, 2], [3, 4]];
var b = [[5, 6], [7, 8]];
var c = matrixMultiply(a, b);
print(c[0][0], c[0][1]);  // 19  22
print(c[1][0], c[1][1]);  // 43  50
```

---

## 10. Real-World Examples

### 10.1 To-Do List Manager

```izilang
class TodoList {
    fn init() {
        this.items = [];
        this.nextId = 1;
    }

    fn add(text) {
        var item = {
            id: this.nextId,
            text: text,
            done: false
        };
        push(this.items, item);
        this.nextId = this.nextId + 1;
        return item;
    }

    fn complete(id) {
        for (var i = 0; i < len(this.items); i = i + 1) {
            if (this.items[i].id == id) {
                this.items[i].done = true;
                return true;
            }
        }
        return false;
    }

    fn remove(id) {
        var result = [];
        for (var i = 0; i < len(this.items); i = i + 1) {
            if (this.items[i].id != id) {
                push(result, this.items[i]);
            }
        }
        this.items = result;
    }

    fn listPending() {
        var result = [];
        for (var i = 0; i < len(this.items); i = i + 1) {
            if (!this.items[i].done) {
                push(result, this.items[i]);
            }
        }
        return result;
    }

    fn printAll() {
        for (var i = 0; i < len(this.items); i = i + 1) {
            var item = this.items[i];
            var status = match item.done {
                true  => "[x]",
                false => "[ ]"
            };
            print(status, item.id + ".", item.text);
        }
    }
}

var todo = TodoList();
todo.add("Buy groceries");
todo.add("Write documentation");
todo.add("Fix the bug");
todo.add("Deploy to production");

todo.complete(1);
todo.complete(3);
todo.remove(4);

todo.printAll();
// [x] 1. Buy groceries
// [ ] 2. Write documentation
// [x] 3. Fix the bug

var pending = todo.listPending();
print("\nPending tasks:", len(pending));  // Pending tasks: 1
```

---

### 10.2 Simple Calculator

```izilang
fn evaluate(expr) {
    import "string";

    // Remove whitespace
    expr = string.replace(expr, " ", "");

    // Try to find + or - (lowest precedence, rightmost to handle left-assoc)
    var depth = 0;
    var lastPlus = -1;
    var lastMinus = -1;

    for (var i = 0; i < len(expr); i = i + 1) {
        var ch = string.substring(expr, i, 1);
        if (ch == "(") { depth = depth + 1; }
        if (ch == ")") { depth = depth - 1; }
        if (depth == 0) {
            if (ch == "+" and i > 0) { lastPlus = i; }
            if (ch == "-" and i > 0) { lastMinus = i; }
        }
    }

    var splitAt = -1;
    var op = "";
    if (lastPlus > lastMinus) { splitAt = lastPlus; op = "+"; }
    else if (lastMinus > lastPlus) { splitAt = lastMinus; op = "-"; }

    if (splitAt >= 0) {
        var left = evaluate(string.substring(expr, 0, splitAt));
        var right = evaluate(string.substring(expr, splitAt + 1, len(expr) - splitAt - 1));
        if (op == "+") { return left + right; }
        if (op == "-") { return left - right; }
    }

    // Remove surrounding parentheses
    if (string.substring(expr, 0, 1) == "(") {
        return evaluate(string.substring(expr, 1, len(expr) - 2));
    }

    return toNumber(expr);
}

// Simple expressions (no * or / for brevity)
print(evaluate("1 + 2"));       // 3
print(evaluate("10 - 3 + 2"));  // 9
print(evaluate("(5 + 3)"));     // 8
```

---

### 10.3 Word Frequency Counter

```izilang
import "string";
import { sort, reverse } from "array";

fn wordFrequency(text) {
    var words = string.split(string.toLower(text), " ");
    var freq = {};

    for (var i = 0; i < len(words); i = i + 1) {
        var word = string.trim(words[i]);
        // Remove punctuation (simple version)
        word = string.replace(word, ".", "");
        word = string.replace(word, ",", "");
        word = string.replace(word, "!", "");
        word = string.replace(word, "?", "");

        if (len(word) > 0) {
            if (hasKey(freq, word)) {
                freq[word] = freq[word] + 1;
            } else {
                freq[word] = 1;
            }
        }
    }
    return freq;
}

fn topN(freq, n) {
    var wordList = keys(freq);
    // Simple insertion sort by frequency descending
    for (var i = 1; i < len(wordList); i = i + 1) {
        var key = wordList[i];
        var j = i - 1;
        while (j >= 0 and freq[wordList[j]] < freq[key]) {
            wordList[j + 1] = wordList[j];
            j = j - 1;
        }
        wordList[j + 1] = key;
    }

    var result = [];
    var limit = n;
    if (len(wordList) < limit) { limit = len(wordList); }
    for (var i = 0; i < limit; i = i + 1) {
        push(result, {word: wordList[i], count: freq[wordList[i]]});
    }
    return result;
}

var text = "to be or not to be that is the question to be is to do";
var freq = wordFrequency(text);
var top5 = topN(freq, 5);

print("Top 5 words:");
for (var i = 0; i < len(top5); i = i + 1) {
    print("  " + top5[i].word + ": " + toString(top5[i].count));
}
// Top 5 words:
//   to: 4
//   be: 3
//   is: 2
//   or: 1
//   not: 1
```

---

### 10.4 Mini Test Runner

Build your own lightweight test framework:

```izilang
class TestRunner {
    fn init(suiteName) {
        this.suiteName = suiteName;
        this.passed = 0;
        this.failed = 0;
        this.failures = [];
    }

    fn test(name, fn) {
        try {
            fn();
            this.passed = this.passed + 1;
            print("  ✓ " + name);
        } catch (e) {
            this.failed = this.failed + 1;
            push(this.failures, {name: name, error: e});
            print("  ✗ " + name + " — " + toString(e));
        }
    }

    fn expect(actual, expected) {
        if (actual != expected) {
            throw "Expected " + toString(expected) + " but got " + toString(actual);
        }
    }

    fn summary() {
        print("\n" + this.suiteName + " Results:");
        print("  Passed: " + toString(this.passed));
        print("  Failed: " + toString(this.failed));
        if (this.failed > 0) {
            print("\nFailures:");
            for (var i = 0; i < len(this.failures); i = i + 1) {
                print("  - " + this.failures[i].name + ": " + toString(this.failures[i].error));
            }
        }
    }
}

// Define the functions to test
fn add(a, b) { return a + b; }
fn factorial(n) {
    if (n <= 1) { return 1; }
    return n * factorial(n - 1);
}

// Run tests
var t = TestRunner("Math Tests");

t.test("add: 1 + 2 = 3", fn() {
    t.expect(add(1, 2), 3);
});

t.test("add: negative numbers", fn() {
    t.expect(add(-1, -2), -3);
});

t.test("factorial: 5! = 120", fn() {
    t.expect(factorial(5), 120);
});

t.test("factorial: 0! = 1", fn() {
    t.expect(factorial(0), 1);
});

t.test("this test fails on purpose", fn() {
    t.expect(1 + 1, 3);  // wrong!
});

t.summary();
```

---

## Best Practices

### Use Descriptive Names

```izilang
// Bad
fn f(x) { return x * 2; }
var d = {n: "Alice", a: 30};

// Good
fn double(value) { return value * 2; }
var user = {name: "Alice", age: 30};
```

### Keep Functions Small and Focused

Each function should do one thing. If a function is getting long, split it:

```izilang
// Instead of one big function:
fn processUserRegistration(data) {
    // 50 lines doing validation, creation, emails, etc.
}

// Split into smaller pieces:
fn validateRegistrationData(data) { /* ... */ }
fn createUserRecord(data) { /* ... */ }
fn sendWelcomeEmail(user) { /* ... */ }
fn processUserRegistration(data) {
    var errors = validateRegistrationData(data);
    if (len(errors) > 0) { return {ok: false, errors: errors}; }
    var user = createUserRecord(data);
    sendWelcomeEmail(user);
    return {ok: true, user: user};
}
```

### Prefer Immutable Data

Use spread to create modified copies instead of mutating:

```izilang
// Mutation (harder to reason about)
var config = {host: "localhost", port: 3000};
config.port = 8080;

// Immutable update (clearer intent)
var config = {host: "localhost", port: 3000};
var updatedConfig = {...config, port: 8080};
```

### Handle Errors Explicitly

Don't ignore possible errors:

```izilang
// Risky: assumes file exists
var content = io.readFile("config.json");

// Better: check first
import "io";
if (io.exists("config.json")) {
    var content = io.readFile("config.json");
    // use content
} else {
    // use defaults
}
```

---

*See also: [Standard Library Reference](STANDARD_LIBRARY.md) | [Tutorial](TUTORIAL.md) | [Language Spec](LANGUAGE_SPEC.md)*

# assert — Runtime Assertions

The `assert` module provides assertion helpers for defensive programming and testing.

## Import

```izilang
// Simple import
import "assert";
assert.ok(x > 0);

// Namespaced wildcard
import * as assert from "std.assert";
assert.ok(x > 0);
assert.eq(a, b);
```

## Functions

### `ok(condition, message?)`

Asserts that `condition` is truthy. Throws a runtime error if the condition is falsy. An optional `message` is included in the error.

```izilang
import * as assert from "std.assert";

assert.ok(true);            // passes
assert.ok(5 > 3);           // passes
assert.ok(false);           // throws: "Assertion failed"
assert.ok(false, "x must be positive");  // throws: "x must be positive"
```

### `eq(a, b)`

Asserts that `a` and `b` are equal (`==`). Throws a runtime error if they are not.

```izilang
import * as assert from "std.assert";

assert.eq(1 + 1, 2);        // passes
assert.eq("hello", "hello"); // passes
assert.eq(1, 2);             // throws: "Expected 1 to equal 2"
```

### `ne(a, b)`

Asserts that `a` and `b` are **not** equal (`!=`). Throws a runtime error if they are equal.

```izilang
import * as assert from "std.assert";

assert.ne(1, 2);        // passes
assert.ne("a", "b");    // passes
assert.ne(1, 1);        // throws: "Expected values to differ"
```

## Complete Example

```izilang
import * as assert from "std.assert";

// Test a math function
fn square(x) {
    return x * x;
}

assert.eq(square(3), 9);
assert.eq(square(0), 0);
assert.ok(square(5) > 20, "5^2 should be greater than 20");
assert.ne(square(3), square(4));

print("All assertions passed!");

// Catching assertion failures
try {
    assert.ok(false, "This will fail");
} catch (e) {
    print("Caught:", e);  // Caught: This will fail
}
```

## Using Assertions in Tests

Assertions are the building block of test suites in IziLang:

```izilang
import * as assert from "std.assert";

fn testAdd() {
    assert.eq(1 + 1, 2);
    assert.eq(0 + 0, 0);
    assert.eq(-1 + 1, 0);
    print("testAdd passed");
}

fn testString() {
    import "string";
    assert.eq(string.toUpper("hello"), "HELLO");
    assert.eq(string.trim("  hi  "), "hi");
    print("testString passed");
}

testAdd();
testString();
```

## See Also

- [Standard Library Index](README.md)
- [Testing Guide](../TESTING.md)

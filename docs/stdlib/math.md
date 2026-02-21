# math — Mathematical Functions

The `math` module provides mathematical constants and functions for numerical computations.

## Import

```izilang
// Module object
import "math";
var result = math.sqrt(16);  // 4

// Named imports
import { sqrt, sin, cos, pi } from "math";
var result = sqrt(16);

// Aliased wildcard
import * as m from "math";
var result = m.sqrt(16);
```

## Constants

| Name | Value | Description |
|------|-------|-------------|
| `pi` / `PI` | `3.14159265358979…` | The mathematical constant π |
| `e` / `E` | `2.71828182845905…` | Euler's number |

## Functions

### Power and Roots

#### `sqrt(x)`

Returns the square root of `x`. `x` must be non-negative.

```izilang
import { sqrt } from "math";
print(sqrt(9));   // 3
print(sqrt(2));   // 1.41421356...
```

#### `pow(base, exp)`

Returns `base` raised to the power `exp`.

```izilang
import { pow } from "math";
print(pow(2, 10));  // 1024
print(pow(3, 3));   // 27
```

### Rounding

#### `floor(x)`

Rounds `x` down to the nearest integer.

```izilang
print(math.floor(3.7));   // 3
print(math.floor(-1.2));  // -2
```

#### `ceil(x)`

Rounds `x` up to the nearest integer.

```izilang
print(math.ceil(3.2));   // 4
print(math.ceil(-1.8));  // -1
```

#### `round(x)`

Rounds `x` to the nearest integer (half rounds up).

```izilang
print(math.round(3.5));  // 4
print(math.round(3.4));  // 3
```

#### `abs(x)`

Returns the absolute value of `x`.

```izilang
print(math.abs(-5));  // 5
print(math.abs(5));   // 5
```

### Trigonometry

Angles are in **radians**. Convert degrees to radians with `deg * pi / 180`.

#### `sin(x)`

```izilang
import { sin, pi } from "math";
print(sin(pi / 2));  // 1
print(sin(0));       // 0
```

#### `cos(x)`

```izilang
import { cos, pi } from "math";
print(cos(0));   // 1
print(cos(pi));  // -1
```

#### `tan(x)`

```izilang
import { tan, pi } from "math";
print(tan(pi / 4));  // 1 (approximately)
```

### Min / Max

#### `min(...values)`

Returns the smallest of the given values (variadic — accepts any number of arguments).

```izilang
print(math.min(3, 1, 4, 1, 5));  // 1
print(math.min(10, 20));          // 10
```

#### `max(...values)`

Returns the largest of the given values (variadic).

```izilang
print(math.max(3, 1, 4, 1, 5));  // 5
print(math.max(10, 20));          // 20
```

## Complete Example

```izilang
import "math";

// Constants
print("pi =", math.pi);   // pi = 3.14159265358979
print("e  =", math.e);    // e  = 2.71828182845905

// Basic functions
print(math.sqrt(25));      // 5
print(math.pow(2, 8));     // 256
print(math.abs(-42));      // 42

// Rounding
print(math.floor(2.9));    // 2
print(math.ceil(2.1));     // 3
print(math.round(2.5));    // 3

// Trigonometry
var angle = math.pi / 6;
print(math.sin(angle));    // 0.5
print(math.cos(angle));    // 0.866025...

// Min / Max
print(math.min(5, 3, 8, 1, 9));  // 1
print(math.max(5, 3, 8, 1, 9));  // 9

// Circle area
fn circleArea(r) {
    return math.pi * math.pow(r, 2);
}
print(circleArea(5));  // 78.5398...
```

## See Also

- [Standard Library Index](README.md)

# array — Array Utilities

The `array` module provides functions for array manipulation and functional programming.

## Import

```izilang
// Module object
import "array";
var doubled = array.map([1, 2, 3], fn(x) { return x * 2; });

// Named imports
import { map, filter, reduce } from "array";
```

## Functions

### Higher-Order Functions

#### `map(array, fn)`

Returns a new array with `fn` applied to each element of `array`. The original array is not modified.

```izilang
import { map } from "array";
var nums = [1, 2, 3, 4, 5];
print(map(nums, fn(x) { return x * 2; }));  // [2, 4, 6, 8, 10]
```

#### `filter(array, fn)`

Returns a new array containing only the elements for which `fn` returns a truthy value.

```izilang
import { filter } from "array";
var nums = [1, 2, 3, 4, 5, 6];
print(filter(nums, fn(x) { return x % 2 == 0; }));  // [2, 4, 6]
```

#### `reduce(array, fn, initial?)`

Reduces `array` to a single value by calling `fn(accumulator, element)` for each element. If `initial` is provided it is used as the starting accumulator; otherwise the first element of the array is used.

```izilang
import { reduce } from "array";
var nums = [1, 2, 3, 4, 5];
var sum = reduce(nums, fn(acc, x) { return acc + x; }, 0);
print(sum);  // 15
```

### Sorting and Ordering

#### `sort(array)`

Returns a new **sorted** array. Works with both numbers and strings. Does not mutate the original.

```izilang
import { sort } from "array";
print(sort([3, 1, 4, 1, 5, 9]));          // [1, 1, 3, 4, 5, 9]
print(sort(["banana", "apple", "cherry"])); // [apple, banana, cherry]
```

#### `reverse(array)`

Returns a new array with the elements in reverse order. Does not mutate the original.

```izilang
import { reverse } from "array";
print(reverse([1, 2, 3, 4, 5]));  // [5, 4, 3, 2, 1]
```

### Combining Arrays

#### `concat(array1, array2)`

Returns a new array that is the concatenation of `array1` and `array2`.

```izilang
import { concat } from "array";
print(concat([1, 2], [3, 4]));  // [1, 2, 3, 4]
```

#### `slice(array, start, end?)`

Returns a new array containing elements from index `start` up to (but not including) index `end`. If `end` is omitted, slices to the end of the array.

```izilang
import { slice } from "array";
var arr = [10, 20, 30, 40, 50];
print(slice(arr, 1, 3));  // [20, 30]
print(slice(arr, 2));     // [30, 40, 50]
```

### Mutation

These functions modify the array **in place**:

#### `push(array, element)`

Appends `element` to the end of `array`. Returns the modified array.

```izilang
var arr = [1, 2, 3];
push(arr, 4);   // arr is now [1, 2, 3, 4]
```

> **Note**: `push` and `pop` are also available as global built-ins without any import.

#### `pop(array)`

Removes and returns the last element of `array`.

```izilang
var arr = [1, 2, 3];
var last = pop(arr);  // returns 3, arr is now [1, 2]
```

#### `len(array)`

Returns the number of elements in `array`.

```izilang
import { len } from "array";
print(len([1, 2, 3]));  // 3
print(len([]));          // 0
```

> **Note**: `len` is also available as a global built-in.

## Complete Example

```izilang
import { map, filter, reduce, sort, reverse, concat, slice } from "array";

var numbers = [5, 3, 8, 1, 9, 2, 7, 4, 6];

// Transform
var doubled = map(numbers, fn(x) { return x * 2; });
print(doubled);  // [10, 6, 16, 2, 18, 4, 14, 8, 12]

// Filter
var evens = filter(numbers, fn(x) { return x % 2 == 0; });
print(evens);  // [8, 2, 4, 6]

// Accumulate
var total = reduce(numbers, fn(acc, x) { return acc + x; }, 0);
print(total);  // 45

// Sort and Reverse
print(sort(numbers));     // [1, 2, 3, 4, 5, 6, 7, 8, 9]
print(reverse([1, 2, 3])); // [3, 2, 1]

// Combine
var a = [1, 2, 3];
var b = [4, 5, 6];
print(concat(a, b));          // [1, 2, 3, 4, 5, 6]
print(slice(concat(a, b), 2, 5));  // [3, 4, 5]

// Pipeline (map → filter → reduce)
var result = reduce(
    filter(
        map([1, 2, 3, 4, 5], fn(x) { return x * x; }),
        fn(x) { return x > 5; }
    ),
    fn(acc, x) { return acc + x; },
    0
);
print(result);  // 16 + 25 = 41
```

## Notes

- `map`, `filter`, `sort`, `reverse`, `concat`, `slice` return **new arrays** and do not mutate the input.
- `push` and `pop` **mutate** the array in place.
- `push` and `pop` are also available as global built-ins.

## See Also

- [Standard Library Index](README.md)
- [string module](string.md) — `join` to turn arrays into strings

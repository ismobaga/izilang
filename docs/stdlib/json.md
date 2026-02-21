# json — JSON Parsing and Serialization

The `json` module provides functions to parse JSON strings into IziLang values and to serialize IziLang values back to JSON.

## Import

```izilang
import * as json from "std.json";
var data = json.parse('{"name": "IziLang"}');
var str  = json.stringify(data);

// Named imports
import { parse, stringify } from "std.json";
```

## Functions

### `parse(jsonString)`

Parses a JSON string and returns the corresponding IziLang value. Throws a runtime error if the input is not valid JSON.

```izilang
import * as json from "std.json";

var num  = json.parse("42");            // 42
var flag = json.parse("true");          // true
var arr  = json.parse('[1, 2, 3]');     // [1, 2, 3]
var obj  = json.parse('{"a": 1}');      // {a: 1}
var none = json.parse("null");          // nil
```

### `stringify(value)`

Converts an IziLang value to its JSON string representation.

```izilang
import * as json from "std.json";

print(json.stringify(nil));             // "null"
print(json.stringify(true));            // "true"
print(json.stringify(42));              // "42"
print(json.stringify("hello"));         // "\"hello\""
print(json.stringify([1, 2, 3]));       // "[1,2,3]"
print(json.stringify({"a": 1}));        // "{\"a\":1}"
```

## Type Mapping

| IziLang Type | JSON Type | Stringify Example |
|---|---|---|
| `nil` | `null` | `"null"` |
| `bool` | `boolean` | `"true"` / `"false"` |
| `number` | `number` | `"42"` / `"3.14"` |
| `string` | `string` | `"\"hello\""` |
| `Array` | `array` | `"[1,2,3]"` |
| `Map` | `object` | `"{\"key\":\"val\"}"` |

## Complete Example

```izilang
import * as json from "std.json";

// Parse a complex object
var raw = '{"users": [{"name": "Alice", "age": 30}, {"name": "Bob", "age": 25}], "count": 2}';
var data = json.parse(raw);

print(data["count"]);           // 2
print(data["users"][0]["name"]); // Alice
print(data["users"][1]["age"]);  // 25

// Modify and re-serialize
data["count"] = 3;
push(data["users"], {"name": "Carol", "age": 28});

var updated = json.stringify(data);
print(updated);
// {"users":[{"name":"Alice","age":30},{"name":"Bob","age":25},{"name":"Carol","age":28}],"count":3}

// Round-trip
var original = {"key": "value", "nums": [1, 2, 3]};
var roundTrip = json.parse(json.stringify(original));
print(roundTrip["key"]);   // "value"
print(roundTrip["nums"]);  // [1, 2, 3]
```

## Error Handling

```izilang
import * as json from "std.json";

try {
    var bad = json.parse("{invalid json}");
} catch (e) {
    print("Parse error:", e);
}
```

## Notes

- JSON parsing is **strict** — invalid JSON throws a runtime error.
- `NaN` and `Infinity` numbers are serialized as `null`.
- Object keys must be strings in JSON; map keys are converted to strings during `stringify`.
- Circular references in maps or arrays are not supported and may cause undefined behavior.

## See Also

- [Standard Library Index](README.md)
- [http module](http.md) — HTTP requests often return JSON bodies
- [regex module](regex.md) — Pattern matching on JSON strings before parsing

# time — Time and Sleep

The `time` module provides functions for getting the current time, sleeping, and formatting timestamps.

## Import

```izilang
import * as time from "std.time";
var now = time.now();
time.sleep(1.5);

// Named imports
import { now, sleep, format } from "std.time";
```

## Functions

### `now()`

Returns the current Unix timestamp in seconds as a floating-point number with millisecond precision.

```izilang
import * as time from "std.time";

var start = time.now();
// ... do some work ...
var elapsed = time.now() - start;
print("Elapsed:", elapsed, "seconds");
```

### `sleep(seconds)`

Pauses execution for `seconds` seconds. Fractional seconds are supported.

```izilang
import * as time from "std.time";

print("Starting...");
time.sleep(1.5);   // sleep for 1.5 seconds
print("Done!");

time.sleep(0.1);   // 100 milliseconds
```

### `format(timestamp, format?)`

Formats a Unix timestamp as a human-readable string. If `format` is omitted, the default format `"%Y-%m-%d %H:%M:%S"` is used.

```izilang
import * as time from "std.time";

var ts = time.now();
print(time.format(ts));                  // "2026-03-15 14:30:00"
print(time.format(ts, "%Y-%m-%d"));      // "2026-03-15"
print(time.format(ts, "%H:%M:%S"));      // "14:30:00"
print(time.format(ts, "%d/%m/%Y"));      // "15/03/2026"
```

#### Format Specifiers

Standard C `strftime` format strings are accepted:

| Specifier | Description | Example |
|-----------|-------------|---------|
| `%Y` | 4-digit year | `2026` |
| `%m` | Month (01–12) | `03` |
| `%d` | Day of month (01–31) | `15` |
| `%H` | Hour, 24-hour (00–23) | `14` |
| `%M` | Minute (00–59) | `30` |
| `%S` | Second (00–59) | `00` |
| `%A` | Full weekday name | `Sunday` |
| `%B` | Full month name | `March` |

## Complete Example

```izilang
import * as time from "std.time";

// Measure execution time
fn benchmark(fn_to_time, iterations) {
    var start = time.now();
    var i = 0;
    while (i < iterations) {
        fn_to_time();
        i = i + 1;
    }
    var elapsed = time.now() - start;
    return elapsed;
}

fn heavyWork() {
    var sum = 0;
    var j = 0;
    while (j < 10000) {
        sum = sum + j;
        j = j + 1;
    }
    return sum;
}

var secs = benchmark(heavyWork, 100);
print("100 iterations took:", secs, "seconds");

// Log with timestamp
fn log(message) {
    var ts = time.format(time.now(), "%H:%M:%S");
    print("[" + ts + "]", message);
}

log("Application started");
time.sleep(0.5);
log("Processing complete");
```

## See Also

- [Standard Library Index](README.md)

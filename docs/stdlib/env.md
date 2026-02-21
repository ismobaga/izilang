# env — Environment Variables

The `env` module provides access to operating system environment variables.

## Import

```izilang
import * as env from "std.env";
var home = env.get("HOME");

// Named imports
import { get, set, exists } from "std.env";
var path = get("PATH");
```

## Functions

### `get(name)`

Returns the value of the environment variable `name` as a string, or `nil` if the variable does not exist.

```izilang
import * as env from "std.env";

var home = env.get("HOME");
if (home != nil) {
    print("Home directory:", home);
} else {
    print("HOME is not set");
}
```

### `set(name, value)`

Sets the environment variable `name` to `value`. Both arguments must be strings. Changes affect the current process and any child processes it spawns.

```izilang
import * as env from "std.env";

env.set("APP_MODE", "production");
env.set("DEBUG_LEVEL", "2");
print(env.get("APP_MODE"));  // "production"
```

### `exists(name)`

Returns `true` if the environment variable `name` exists, `false` otherwise.

```izilang
import * as env from "std.env";

if (env.exists("DEBUG")) {
    print("Debug mode is on");
}
```

## Complete Example

```izilang
import * as env from "std.env";

// Read common system variables
var home = env.get("HOME");
var user = env.get("USER");
var path = env.get("PATH");

print("User:", user);
print("Home:", home);

// Check for a feature flag
if (env.exists("MY_APP_VERBOSE")) {
    print("Verbose output enabled");
}

// Set and retrieve custom variables
env.set("MY_APP_VERSION", "1.0.0");
var version = env.get("MY_APP_VERSION");
print("Version:", version);  // "1.0.0"

// Gracefully handle missing variables
var dbUrl = env.get("DATABASE_URL");
if (dbUrl == nil) {
    dbUrl = "sqlite://./local.db";
    print("Using default database:", dbUrl);
}
```

## Platform Notes

- Reading uses POSIX `getenv()` (available on all platforms).
- Writing uses `setenv()` on Linux/macOS and `_putenv_s()` on Windows.
- Variable names and values are **case-sensitive** on Linux/macOS; case-insensitive on Windows.
- Environment variable changes only affect the current process and its children.

## See Also

- [Standard Library Index](README.md)
- [io module](io.md) — File operations

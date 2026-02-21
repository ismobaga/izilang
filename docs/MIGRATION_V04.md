# Migration Guide: v0.3 → v0.4

## Introduction

IziLang v0.4 is **100% backward compatible** with v0.3. All existing code will continue to run without modification. This guide describes the **new features** added in v0.4 and how to adopt them.

---

## Quick Summary

### What's New in v0.4

| Feature | Description |
|---------|-------------|
| ✨ **Async/Await** | Cooperative single-threaded async functions |
| ✨ **OS Threads** | True parallel execution with `thread_spawn` |
| ✨ **Mutex** | Synchronisation primitives for shared state |
| ✨ **`std.ipc` module** | Named-pipe IPC between processes |
| ✨ **`std.time` module** | Timestamps, sleep, and date formatting |
| ✨ **`std.json` module** | JSON parsing and serialisation |
| ✨ **`std.regex` module** | Regular expression matching and replacement |
| ✨ **`std.env` module** | Environment variable access |
| ✨ **`std.http` module** | Synchronous HTTP client |
| ✨ **`ui` module** | Graphical window and drawing (raylib) |
| ✨ **Package manager** | `izi-pkg` CLI for dependency management |
| ✨ **Stack overflow protection** | Deep recursion is detected and reported |
| ✨ **`izi bench`** | Built-in benchmark command |
| ✨ **REPL improvements** | `:vars`, `:tasks`, persistent state |

### Breaking Changes

**None.** All v0.3 code runs unchanged in v0.4.

---

## Migration Steps

### Step 1: Verify Compatibility

Your existing code should work as-is:

```bash
izi run yourapp.iz
```

If any issues arise, please report them as bugs.

### Step 2: Adopt the New Standard Library Modules

v0.4 adds several new standard library modules. Previously you may have used workarounds for functionality now provided natively.

#### Before (v0.3): Manual JSON handling

```izilang
// Manually building JSON strings (fragile)
var json = "{\"name\": \"" + name + "\", \"age\": " + toString(age) + "}";
```

#### After (v0.4): Use `std.json`

```izilang
import * as json from "std.json";
var data = {"name": name, "age": age};
var jsonStr = json.stringify(data);
```

#### Before (v0.3): Using `clock()` for timestamps

```izilang
var start = clock();
```

#### After (v0.4): Use `std.time`

```izilang
import * as time from "std.time";
var start = time.now();
// time.sleep(1.0) and time.format(ts) also available
```

### Step 3: Adopt Async/Await (Optional)

If your application performs I/O-bound work, consider using async/await for cleaner sequencing:

#### Before (v0.3): Synchronous blocking calls

```izilang
import "io";
var content = io.readFile("data.txt");
processData(content);
```

#### After (v0.4): Async pipeline

```izilang
import "io";

async fn readAndProcess(path) {
    var content = await io.readFile(path);
    return processData(content);
}

var result = await readAndProcess("data.txt");
```

> Async/await is entirely optional. Your synchronous code is still idiomatic IziLang.

### Step 4: Adopt Threads for CPU-Bound Work (Optional)

For computationally intensive workloads, `thread_spawn` enables true parallelism:

#### Before (v0.3): Sequential computation

```izilang
fn processItem(item) { return item * item; }

var results = [];
var i = 0;
while (i < len(items)) {
    push(results, processItem(items[i]));
    i = i + 1;
}
```

#### After (v0.4): Parallel computation

```izilang
fn processItem(item) { return item * item; }

var threads = [];
var i = 0;
while (i < len(items)) {
    var item = items[i];
    push(threads, thread_spawn(fn() { return processItem(item); }));
    i = i + 1;
}

var results = [];
var j = 0;
while (j < len(threads)) {
    push(results, await threads[j]);
    j = j + 1;
}
```

### Step 5: Replace Workarounds with `std.regex`

If you used manual string searching, `std.regex` provides richer pattern matching:

#### Before (v0.3)

```izilang
import "string";
var found = string.indexOf(text, "error") != -1;
```

#### After (v0.4)

```izilang
import * as regex from "std.regex";
var found = regex.test(text, "(?i)error");  // case-insensitive
```

---

## New Feature Details

### Async/Await

```izilang
// Declare an async function — calling it returns a Task
async fn greet(name) {
    return "Hello, " + name + "!";
}

// await runs the task and returns the result
var msg = await greet("Alice");
print(msg);  // "Hello, Alice!"
```

See the [Concurrency Guide](CONCURRENCY.md) for the full documentation.

### OS Threads

```izilang
// Spawn a thread — runs in parallel
var t1 = thread_spawn(fn() { return 100 * 100; });
var t2 = thread_spawn(fn() { return 200 * 200; });

print(await t1 + await t2);  // 50000
```

### Mutex

```izilang
var counter = [0];
var m = mutex();

var workers = [];
var i = 0;
while (i < 8) {
    push(workers, thread_spawn(fn() {
        lock(m);
        counter[0] = counter[0] + 1;
        unlock(m);
    }));
    i = i + 1;
}

while (len(workers) > 0) {
    await pop(workers);
}

print(counter[0]);  // 8
```

### `std.ipc` Module

```izilang
import * as ipc from "std.ipc";

ipc.createPipe("ch");
var r = ipc.openRead("ch");
var w = ipc.openWrite("ch");

ipc.send(w, "hello");
print(ipc.recv(r));  // "hello"

ipc.close(w);
ipc.close(r);
ipc.removePipe("ch");
```

See the [`ipc` module reference](stdlib/ipc.md).

### `std.json` Module

```izilang
import * as json from "std.json";

var obj = json.parse('{"name": "IziLang", "version": 0.4}');
print(obj["name"]);     // IziLang
print(obj["version"]);  // 0.4

print(json.stringify({"ok": true}));  // '{"ok":true}'
```

See the [`json` module reference](stdlib/json.md).

### `std.time` Module

```izilang
import * as time from "std.time";

var start = time.now();
time.sleep(0.5);
var elapsed = time.now() - start;
print("Elapsed:", elapsed, "seconds");

print(time.format(time.now(), "%Y-%m-%d"));  // "2026-03-15"
```

See the [`time` module reference](stdlib/time.md).

### `std.env` Module

```izilang
import * as env from "std.env";

var home = env.get("HOME");
env.set("APP_MODE", "production");
print(env.exists("PATH"));  // true
```

See the [`env` module reference](stdlib/env.md).

### `std.regex` Module

```izilang
import * as regex from "std.regex";

print(regex.test("Hello World", "\\bWorld\\b"));  // true
print(regex.replace("2026-03-15", "(\\d{4})-(\\d{2})-(\\d{2})", "$3/$2/$1"));
// "15/03/2026"
```

See the [`regex` module reference](stdlib/regex.md).

### Package Manager (`izi-pkg`)

v0.4 ships a minimal package manager. Add dependencies to your project:

```bash
# Add a local library
izi-pkg add path=../mylib

# Add a git repository
izi-pkg add git=https://github.com/example/izi-utils@main

# Resolve and install all dependencies
izi-pkg sync
```

Dependencies are installed into `libs/` and a content-hash lock file (`izi.lock`) is generated.

See the [Package Manager Guide](PACKAGE_MANAGER.md) for full documentation.

### Stack Overflow Protection

Recursive programs that exceed the call depth limit (default: 256) now receive a clear error instead of a segfault:

```
RuntimeError: Stack overflow: call depth limit (256) exceeded.
```

Avoid deep recursion or refactor to use an iterative approach.

### `izi bench` Command

Benchmark your code directly:

```bash
izi bench myscript.iz --iterations 1000
```

---

## Feature Comparison

| Feature | v0.3 | v0.4 |
|---------|------|------|
| Async/await | ❌ | ✅ |
| OS threads | ❌ | ✅ |
| Mutex | ❌ | ✅ |
| `std.ipc` | ❌ | ✅ |
| `std.json` | ❌ | ✅ |
| `std.time` | ❌ | ✅ |
| `std.env` | ❌ | ✅ |
| `std.regex` | ❌ | ✅ |
| `std.http` | ❌ | ✅ |
| `ui` (raylib) | ❌ | ✅ |
| Package manager | ❌ | ✅ MVP |
| Stack overflow detection | ❌ | ✅ |
| `izi bench` | ❌ | ✅ |
| Gradual typing | ✅ | ✅ |
| Classes / OOP | ✅ | ✅ |
| Mark-and-sweep GC | ✅ | ✅ |
| Backward compatibility | — | ✅ 100% |

---

## Deprecations

### None in v0.4

IziLang v0.4 introduces no deprecations. All v0.3 features remain fully supported.

---

## Known Limitations (v0.4)

- `regex.match()` is disabled due to a bug; use `regex.test()` and `regex.replace()` instead.
- The HTTP client supports plain HTTP only; HTTPS is not yet available.
- `std.ipc` is supported on Linux and macOS only; Windows support is planned for v0.5.
- Package registry (remote packages) is planned for a later release; v0.4 supports local-path and git dependencies only.

---

## Getting Help

- [v0.4 Standard Library Index](stdlib/README.md)
- [Concurrency Guide](CONCURRENCY.md)
- [CLI App Tutorial](CLI_APP_TUTORIAL.md)
- [GitHub Issues](https://github.com/ismobaga/izilang/issues)

---

**Last Updated**: 2026-02-21  
**Version**: 0.4.0

# Concurrency Guide

IziLang provides three complementary concurrency primitives:

| Primitive | Use Case |
|-----------|----------|
| **Async/Await** | Cooperative, single-threaded asynchronous code |
| **OS Threads** | True parallelism for CPU-bound work |
| **Mutex** | Protect shared mutable state between threads |

---

## 1. Async/Await

Async/await is IziLang's cooperative concurrency model. It is **single-threaded**: only one task runs at a time, but long-running or I/O-bound work can be structured as non-blocking tasks.

### Declaring an Async Function

Prefix a function declaration with `async`:

```izilang
async fn fetchData(url) {
    // Simulate an I/O operation
    return "data from " + url;
}
```

Calling an async function **immediately returns a `Task` object** without executing the body.

### Awaiting a Task

Use `await` to run a task and get its return value:

```izilang
var task = fetchData("https://api.example.com/users");
var result = await task;
print(result);  // "data from https://api.example.com/users"
```

You can also `await` the call directly:

```izilang
var result = await fetchData("https://api.example.com/users");
```

### Chaining Async Calls

```izilang
async fn fetchData(url) {
    return "raw: " + url;
}

async fn processData(raw) {
    return "processed: " + raw;
}

async fn pipeline(url) {
    var raw = await fetchData(url);
    var result = await processData(raw);
    return result;
}

var output = await pipeline("https://api.example.com");
print(output);  // "processed: raw: https://api.example.com"
```

### Async Main Pattern

Wrap application logic in an async `main` function:

```izilang
async fn main() {
    var data = await fetchData("https://api.example.com");
    print(data);
}

await main();
```

---

## 2. OS Threads

`thread_spawn` launches a callable on a real **OS thread**, enabling true parallelism on multi-core hardware.

### Spawning a Thread

```izilang
var t = thread_spawn(fn() {
    return 42 * 42;
});
```

`thread_spawn` returns immediately with a **Task handle**. The thread runs concurrently with the rest of the program.

### Waiting for a Thread

Use `await` to block until the thread finishes and retrieve its return value:

```izilang
var result = await t;
print(result);  // 1764
```

### Parallel Computation

```izilang
// Launch two threads in parallel
var t1 = thread_spawn(fn() { return 10 * 10; });
var t2 = thread_spawn(fn() { return 20 * 20; });

// Wait for both
var r1 = await t1;  // 100
var r2 = await t2;  // 400
print(r1 + r2);     // 500
```

### Fan-Out Pattern

```izilang
// Process multiple items in parallel
var urls = [
    "http://example.com/a",
    "http://example.com/b",
    "http://example.com/c"
];

import { map } from "array";
var tasks = map(urls, fn(url) {
    return thread_spawn(fn() {
        // Simulate fetching each URL
        return "result for " + url;
    });
});

// Collect all results
var results = map(tasks, fn(t) { return await t; });
print(results);
```

---

## 3. Mutex — Protecting Shared State

When multiple threads access the same mutable data, you must protect it with a **mutex** to prevent data races.

### Creating a Mutex

```izilang
var m = mutex();
```

### `lock(m)` and `unlock(m)`

`lock` acquires the mutex, blocking until it is available. Always call `unlock` when done:

```izilang
var counter = [0];
var m = mutex();

var workers = [];
var i = 0;
while (i < 4) {
    push(workers, thread_spawn(fn() {
        lock(m);
        counter[0] = counter[0] + 1;
        unlock(m);
    }));
    i = i + 1;
}

// Wait for all workers to finish
var j = 0;
while (j < len(workers)) {
    await workers[j];
    j = j + 1;
}

print(counter[0]);  // 4
```

> **Important**: Always unlock the mutex, even if an error occurs. Consider wrapping critical sections in a `try/finally` block.

### `trylock(m)`

Attempts to acquire the mutex **without blocking**. Returns `true` if the lock was acquired, `false` if it was already held.

```izilang
var m = mutex();

if (trylock(m)) {
    print("Lock acquired");
    // ... critical section ...
    unlock(m);
} else {
    print("Lock was busy — skipping");
}
```

### Safe Mutex Pattern

Use `try/finally` to ensure the mutex is always released:

```izilang
fn withLock(m, fn_body) {
    lock(m);
    try {
        fn_body();
    } finally {
        unlock(m);
    }
}

var shared = [0];
var m = mutex();

withLock(m, fn() {
    shared[0] = shared[0] + 1;
});
```

---

## 4. Choosing the Right Primitive

| Scenario | Recommended Primitive |
|----------|-----------------------|
| Sequencing I/O without blocking | `async`/`await` |
| CPU-bound parallel work | `thread_spawn` + `await` |
| Shared mutable state between threads | `mutex` + `lock`/`unlock` |
| Non-blocking lock attempt | `trylock` |

### Async vs Threads

- **Async** is suitable for tasks that spend most of their time waiting (I/O, network, timers). It is lightweight because no extra OS threads are created.
- **Threads** are suitable for computationally intensive work that benefits from multiple CPU cores.

---

## 5. Complete Example: Worker Pool

A worker pool distributes tasks across N threads and collects results:

```izilang
fn workerPool(items, worker_fn, num_workers) {
    // Split items into chunks
    var chunk_size = len(items) / num_workers;
    var threads = [];
    var i = 0;

    while (i < num_workers) {
        var start = i * chunk_size;
        var end = start + chunk_size;
        if (i == num_workers - 1) {
            end = len(items);  // last chunk gets the remainder
        }

        import { slice } from "array";
        var chunk = slice(items, start, end);

        push(threads, thread_spawn(fn() {
            import { map } from "array";
            return map(chunk, worker_fn);
        }));

        i = i + 1;
    }

    // Collect results from all workers
    var results = [];
    var j = 0;
    while (j < len(threads)) {
        var partial = await threads[j];
        var k = 0;
        while (k < len(partial)) {
            push(results, partial[k]);
            k = k + 1;
        }
        j = j + 1;
    }
    return results;
}

// Compute squares of 1..20 using 4 threads
var numbers = [];
var n = 1;
while (n <= 20) {
    push(numbers, n);
    n = n + 1;
}

var squares = workerPool(numbers, fn(x) { return x * x; }, 4);
print(squares);
```

---

## 6. Concurrency with IPC

For scenarios requiring separate **processes** (rather than threads), use the [`ipc` module](stdlib/ipc.md):

```izilang
import * as ipc from "std.ipc";

// Parent sends work to a child process over a named pipe
ipc.createPipe("job_queue");
var writer = ipc.openWrite("job_queue");
ipc.send(writer, "process:file1.txt");
ipc.send(writer, "STOP");
ipc.close(writer);
```

See the [ipc module reference](stdlib/ipc.md) for a full example.

---

## See Also

- [Standard Library Index](stdlib/README.md)
- [ipc module](stdlib/ipc.md) — Cross-process messaging
- [Language Tutorial](TUTORIAL.md) — Core language concepts

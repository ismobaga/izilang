# ipc — Inter-Process Communication

The `ipc` module provides named-pipe (FIFO) based primitives for passing messages between processes.

> **Platform Note**: The `std.ipc` module is supported on **Linux** and **macOS** only. Windows is not currently supported and will throw a descriptive error.

## Import

```izilang
import * as ipc from "std.ipc";

// Named imports
import { createPipe, openRead, openWrite, send, recv, tryRecv, close, removePipe } from "std.ipc";
```

## Concepts

- A **pipe** is identified by a logical `name` string. The underlying FIFO is created at `/tmp/izi_ipc_<name>` with permissions `0600`.
- A **handle** is a numeric descriptor returned by `openRead` or `openWrite`. Handles must be closed with `close()` when no longer needed.
- `recv` **blocks** until a message arrives; `tryRecv` returns `nil` immediately if no message is ready.

## Functions

### `createPipe(name)`

Creates a named pipe (FIFO) identified by `name`. Returns `true` on success. Call this before `openRead`/`openWrite`.

```izilang
ipc.createPipe("my_channel");
```

### `openRead(name)`

Opens the read end of a named pipe. Returns a handle (number). **Blocks** until a writer has also opened the pipe.

```izilang
var reader = ipc.openRead("my_channel");
```

### `openWrite(name)`

Opens the write end of a named pipe. Returns a handle (number). Requires an active reader.

```izilang
var writer = ipc.openWrite("my_channel");
```

### `send(handle, message)`

Sends a string `message` through the write `handle`. Returns `true` on success.

```izilang
ipc.send(writer, "Hello, IPC!");
```

### `recv(handle)`

**Blocking** receive from a read `handle`. Returns the message string. Waits until a message is available.

```izilang
var msg = ipc.recv(reader);
print(msg);  // "Hello, IPC!"
```

### `tryRecv(handle)`

**Non-blocking** receive. Returns the message string if one is ready, or `nil` if the pipe is empty.

```izilang
var msg = ipc.tryRecv(reader);
if (msg != nil) {
    print("Received:", msg);
} else {
    print("No message yet");
}
```

### `close(handle)`

Closes the pipe handle and releases resources.

```izilang
ipc.close(writer);
ipc.close(reader);
```

### `removePipe(name)`

Removes the named pipe from the filesystem.

```izilang
ipc.removePipe("my_channel");
```

## Complete Example

```izilang
import * as ipc from "std.ipc";

var ch = "demo_channel";

// Setup
ipc.createPipe(ch);
var reader = ipc.openRead(ch);
var writer = ipc.openWrite(ch);

// Exchange messages
ipc.send(writer, "Message 1");
ipc.send(writer, "Message 2");
ipc.send(writer, "STOP");

var running = true;
while (running) {
    var msg = ipc.recv(reader);
    print("Got:", msg);
    if (msg == "STOP") {
        running = false;
    }
}

// Cleanup
ipc.close(writer);
ipc.close(reader);
ipc.removePipe(ch);
```

## Typical Two-Process Pattern

In a real scenario you would use IPC across two separate processes. Split the setup as follows:

**Process A (sender):**
```izilang
import * as ipc from "std.ipc";
ipc.createPipe("results");
var w = ipc.openWrite("results");
ipc.send(w, "done:42");
ipc.close(w);
```

**Process B (receiver):**
```izilang
import * as ipc from "std.ipc";
var r = ipc.openRead("results");
var msg = ipc.recv(r);
print("Received:", msg);
ipc.close(r);
ipc.removePipe("results");
```

## Security Notes

- Pipes are created with permissions `0600` (owner read/write only).
- Pipe files reside under `/tmp/izi_ipc_<name>`. Sanitize `name` to avoid path traversal.

## See Also

- [Standard Library Index](README.md)
- [Full IPC reference](../IPC.md)
- [`examples/demo_ipc.iz`](../../examples/demo_ipc.iz)

# std.ipc - Inter-Process Communication

The `std.ipc` module provides IPC (Inter-Process Communication) primitives for izilang scripts, enabling message passing between separate processes using named pipes (FIFOs).

## Platform Support

The `std.ipc` module is supported on **Linux** and **macOS** (POSIX systems). Windows is not currently supported.

## Module Import

```izilang
import * as ipc from "std.ipc";
```

Or with named imports:

```izilang
import { createPipe, openRead, openWrite, send, recv, tryRecv, close, removePipe } from "std.ipc";
```

## API Reference

### `ipc.createPipe(name)`

Creates a named pipe (FIFO) identified by `name`. The underlying file is created at `/tmp/izi_ipc_<name>`. If the pipe already exists, the call succeeds silently.

- **Parameters**: `name` (string) — logical name for the pipe
- **Returns**: `true` on success
- **Throws**: runtime error if the pipe cannot be created

```izilang
ipc.createPipe("my_channel");
```

### `ipc.openRead(name)`

Opens a named pipe for reading. Returns a handle (number) that can be passed to `recv`, `tryRecv`, and `close`.

- **Parameters**: `name` (string) — name of the pipe to open
- **Returns**: handle (number)
- **Throws**: runtime error if the pipe does not exist or cannot be opened
- **Note**: Opens immediately without blocking even if no writer is connected yet

```izilang
var reader = ipc.openRead("my_channel");
```

### `ipc.openWrite(name)`

Opens a named pipe for writing. Returns a handle (number) that can be passed to `send` and `close`.

- **Parameters**: `name` (string) — name of the pipe to open
- **Returns**: handle (number)
- **Throws**: runtime error if the pipe does not exist, no reader is connected, or the pipe cannot be opened
- **Note**: Requires that a reader has already opened the pipe (use `openRead` first)

```izilang
var writer = ipc.openWrite("my_channel");
```

### `ipc.send(handle, message)`

Sends a string message through a write handle. Messages use a 4-byte length prefix internally, so they can contain any content including newlines and null bytes.

- **Parameters**:
  - `handle` (number) — a write handle returned by `openWrite`
  - `message` (string) — the message to send
- **Returns**: `true` on success
- **Throws**: runtime error if the handle is invalid, read-only, or the write fails

```izilang
ipc.send(writer, "hello from process A");
```

### `ipc.recv(handle)`

Receives a message from a read handle. This call **blocks** until a complete message is available.

- **Parameters**: `handle` (number) — a read handle returned by `openRead`
- **Returns**: the received message (string)
- **Throws**: runtime error if the handle is invalid, write-only, or the pipe is closed

```izilang
var message = ipc.recv(reader);
print(message);
```

### `ipc.tryRecv(handle)`

Attempts to receive a message from a read handle **without blocking**. Returns `nil` if no message is currently available.

- **Parameters**: `handle` (number) — a read handle returned by `openRead`
- **Returns**: the received message (string) if available, or `nil`
- **Throws**: runtime error if the handle is invalid or write-only

```izilang
var message = ipc.tryRecv(reader);
if (message != nil) {
    print("Got: " + message);
} else {
    print("No message yet");
}
```

### `ipc.close(handle)`

Closes an IPC handle and releases associated resources.

- **Parameters**: `handle` (number) — a handle returned by `openRead` or `openWrite`
- **Returns**: `nil`
- **Throws**: runtime error if the handle is invalid

```izilang
ipc.close(reader);
ipc.close(writer);
```

### `ipc.removePipe(name)`

Removes the named pipe from the filesystem.

- **Parameters**: `name` (string) — name of the pipe to remove
- **Returns**: `true` on success
- **Throws**: runtime error if removal fails (pipe not existing is silently ignored)

```izilang
ipc.removePipe("my_channel");
```

## Usage Pattern

Named pipes are **half-duplex** (one-way): use two pipes for bidirectional communication. The typical lifecycle is:

1. **Setup** (one process, or before forking):
   - Call `createPipe` to create the FIFO on the filesystem
2. **Reader** (receiving process):
   - Call `openRead` to open the read end
3. **Writer** (sending process):
   - Call `openWrite` to open the write end *(after the reader is open)*
4. **Communication**:
   - Writer calls `send`; reader calls `recv` or `tryRecv`
5. **Teardown**:
   - Both sides call `close`
   - One side calls `removePipe` to clean up the filesystem entry

## Example: Single-Process Round-Trip

```izilang
import * as ipc from "std.ipc";

var ch = "demo_channel";
ipc.createPipe(ch);

// Open reader first (non-blocking open)
var reader = ipc.openRead(ch);
// Then open writer (succeeds because reader is already connected)
var writer = ipc.openWrite(ch);

ipc.send(writer, "ping");
var reply = ipc.recv(reader);
print("Received: " + reply);  // Received: ping

ipc.close(writer);
ipc.close(reader);
ipc.removePipe(ch);
```

## Example: Two-Process Communication

**sender.iz** (run first in background or separate terminal):

```izilang
import * as ipc from "std.ipc";

var ch = "shared_channel";
// Pipe must already exist (created by receiver or beforehand)
var writer = ipc.openWrite(ch);
ipc.send(writer, "Hello from sender!");
ipc.close(writer);
```

**receiver.iz**:

```izilang
import * as ipc from "std.ipc";

var ch = "shared_channel";
ipc.createPipe(ch);

var reader = ipc.openRead(ch);
var msg = ipc.recv(reader);
print("Received: " + msg);

ipc.close(reader);
ipc.removePipe(ch);
```

## Security Considerations

- Named pipes are created in `/tmp` with permissions `0600` (owner read/write only).
- Avoid using untrusted strings as pipe names to prevent path traversal or conflicts.
- Pipe names are prefixed with `izi_ipc_` to reduce accidental collisions with other programs.
- For sensitive data, consider using encrypted channels at the application level.

## Limitations

- Currently POSIX-only (Linux and macOS). Windows support is not implemented.
- Named pipes provide **unidirectional** communication; use two pipes for bidirectional messaging.
- Messages are limited to approximately 2 GB (uint32_t length prefix).
- Large messages may be split across multiple `write` syscalls; the implementation handles reassembly.

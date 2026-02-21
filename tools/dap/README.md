# IziLang Debug Adapter Protocol (DAP) Server

This tool implements the [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/) for IziLang, enabling full debugging support in VS Code and other DAP-compatible editors.

## Features

- **Breakpoints** – Set line-based breakpoints in `.iz` source files.
- **Stepping** – Step in, step over, and step out of functions.
- **Variable inspection** – Inspect local and global variables when paused.
- **Call stack** – View the full IziLang call stack at any pause point.
- **Stop on entry** – Optionally pause on the very first statement.

## Building

From the `tools/dap/` directory:

```bash
# Generate makefiles (requires premake5 in PATH or use the one in the repo root)
../../premake5 gmake2

# Build debug version
make config=debug

# The binary is placed at:
# bin/Debug/izilang-dap
```

## Usage

The DAP server communicates over standard I/O in DAP format.

```bash
izilang-dap --stdio
```

It is normally launched automatically by the VS Code extension when a debug session starts.

## VS Code Integration

1. Install the IziLang VS Code extension from `tools/vscode-extension/`.
2. Place `izilang-dap` somewhere on your `PATH`, or set the `izilang.dap.adapterPath` setting.
3. Add a debug configuration to your `launch.json`:

```json
{
  "type": "izilang",
  "request": "launch",
  "name": "Debug IziLang",
  "program": "${file}",
  "stopOnEntry": false
}
```

## Supported DAP Requests

| Request | Support |
|---|---|
| `initialize` | ✅ |
| `launch` | ✅ |
| `setBreakpoints` | ✅ |
| `configurationDone` | ✅ |
| `continue` | ✅ |
| `next` (step over) | ✅ |
| `stepIn` | ✅ |
| `stepOut` | ✅ |
| `pause` | ✅ |
| `stackTrace` | ✅ |
| `scopes` | ✅ |
| `variables` | ✅ |
| `threads` | ✅ |
| `disconnect` | ✅ |
| `attach` | ❌ (not applicable for tree-walking interpreter) |

# IziLang LSP Server

A Language Server Protocol implementation for IziLang.

## Building

```bash
premake5 gmake2
make config=release
```

## Running

```bash
./bin/Release/izilang-lsp --stdio
```

The server communicates over standard input/output using the JSON-RPC protocol.

## Features

### Current Status

- ✅ Basic LSP scaffolding
- ✅ Initialize handshake
- ⬜ Full text synchronization
- ⬜ Diagnostics (errors/warnings)
- ⬜ Code completion
- ⬜ Go to definition
- ⬜ Hover information
- ⬜ Symbol search

### Future Work

This is a minimal skeleton implementation. To make it fully functional:

1. **JSON Library**: Add a JSON parsing library (e.g., nlohmann/json)
2. **Parser Integration**: Use the IziLang parser to analyze code
3. **Symbol Table**: Build and maintain symbol information
4. **Completion Engine**: Generate context-aware completions
5. **Diagnostics**: Report errors and warnings from the parser

## Integration with Editors

### VS Code

The VS Code extension in `tools/vscode-extension` connects to this server automatically.

### Other Editors

Any editor that supports LSP can connect to this server:

- **Vim/Neovim**: Use plugins like coc.nvim or nvim-lspconfig
- **Emacs**: Use lsp-mode
- **Sublime Text**: Use LSP package
- **IntelliJ**: Use LSP Support plugin

## Protocol

The server implements a subset of the [Language Server Protocol](https://microsoft.github.io/language-server-protocol/):

- Lifecycle: initialize, shutdown, exit
- Text sync: didOpen, didChange, didSave
- Language features: completion, hover, definition

## Development

The server is built using the existing IziLang parser infrastructure. Key files:

- `src/main.cpp`: LSP server entry point and message handling
- `../../src/parse/`: Parser for analyzing IziLang code
- `../../src/ast/`: AST definitions for code structure

## Testing

Test the server manually:

```bash
# Start server
./bin/Release/izilang-lsp --stdio

# Send initialize request (copy-paste into stdin)
Content-Length: 150

{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"capabilities":{}}}
```

## See Also

- [LSP Specification](https://microsoft.github.io/language-server-protocol/)
- [IziLang LSP Documentation](../../docs/LSP_SERVER.md)

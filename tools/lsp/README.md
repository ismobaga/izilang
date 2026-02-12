# IziLang LSP Server

A fully-functional Language Server Protocol implementation for IziLang providing IDE features like code completion, diagnostics, hover information, and more.

## Features

### Implemented ✅

- ✅ **Diagnostics**: Real-time syntax errors and semantic warnings
  - Lexer errors (invalid tokens)
  - Parser errors (syntax errors)
  - Semantic errors (duplicate variables, type mismatches)
  - Semantic warnings (unused variables)

- ✅ **Code Completion**: Context-aware suggestions
  - 29 language keywords (var, fn, if, else, while, for, etc.)
  - Local variables and parameters
  - Function names
  - Class names

- ✅ **Hover Information**: Rich documentation on hover
  - Symbol type (variable, function, class, parameter)
  - Definition location (line number)
  - Reference count

- ✅ **Go to Definition**: Navigate to symbol declarations
  - Variables
  - Functions
  - Classes
  - Parameters

- ✅ **Find References**: Locate all usages of a symbol
  - Includes declaration when requested
  - Shows all references across the document

- ✅ **Rename Symbol**: Refactor symbol names safely
  - Renames definition and all references
  - Generates workspace edits

- ✅ **Document Symbols**: Outline view of code structure
  - All functions
  - All variables
  - All classes
  - Function parameters

## Building

```bash
cd tools/lsp
../../premake5 gmake2
make config=release
```

## Running

```bash
./bin/Release/izilang-lsp --stdio
```

The server communicates over standard input/output using the JSON-RPC 2.0 protocol.

## Testing

A comprehensive test script is available to verify all features:

```bash
# Run the test script
python3 test_lsp_full.py
```

Example test output:
```
✓ Go to Definition - Found definition at line 4
✓ Find References - Found 3 references for variable 'x'
✓ Rename Symbol - Generated 2 text edits
✓ Document Symbols - Found 10 symbols
✓ Diagnostics - Found duplicate variable error
```

## Architecture

### Components

- **LSPServer** (`lsp_server.hpp/cpp`): Main server handling JSON-RPC messages
- **DocumentManager** (`document_manager.hpp/cpp`): Manages open documents and their state
- **Document**: Represents a single source file with parsed AST and symbols
- **SymbolTableBuilder**: Visitor that builds symbol tables from AST

### Flow

1. Client opens document → `didOpen` notification
2. Server parses document using IziLang lexer/parser
3. Server runs semantic analyzer for diagnostics
4. Server builds symbol table tracking definitions and references
5. Server publishes diagnostics to client
6. Client requests features (hover, completion, etc.)
7. Server responds using symbol table and AST information

## Integration with Editors

### VS Code

The VS Code extension in `../vscode-extension` connects to this server automatically.

### Other Editors

Any editor that supports LSP can connect to this server:

- **Vim/Neovim**: Use plugins like coc.nvim or nvim-lspconfig
- **Emacs**: Use lsp-mode
- **Sublime Text**: Use LSP package
- **IntelliJ**: Use LSP Support plugin

Configuration example for Neovim:

```lua
require'lspconfig'.configs.izilang = {
  default_config = {
    cmd = {'/path/to/izilang-lsp', '--stdio'},
    filetypes = {'izilang'},
    root_dir = function(fname)
      return vim.fn.getcwd()
    end,
  },
}
require'lspconfig'.izilang.setup{}
```

## Protocol Support

The server implements the following LSP methods:

### Lifecycle
- `initialize` - Initialize server with client capabilities
- `initialized` - Client ready notification
- `shutdown` - Shutdown server
- `exit` - Exit server process

### Text Synchronization
- `textDocument/didOpen` - Document opened
- `textDocument/didChange` - Document changed (full sync)
- `textDocument/didClose` - Document closed
- `textDocument/didSave` - Document saved

### Language Features
- `textDocument/completion` - Code completion
- `textDocument/hover` - Hover information
- `textDocument/definition` - Go to definition
- `textDocument/references` - Find references
- `textDocument/rename` - Rename symbol
- `textDocument/documentSymbol` - Document outline

### Diagnostics
- `textDocument/publishDiagnostics` - Publish errors/warnings (server → client)

## Dependencies

- **nlohmann/json**: JSON library for parsing JSON-RPC messages (included in `third_party/`)
- **IziLang Parser**: Lexer, Parser, and Semantic Analyzer from main project

## Development

### Key Files

- `src/main.cpp`: Entry point and command-line handling
- `src/lsp_server.hpp/cpp`: JSON-RPC message handling and LSP protocol implementation
- `src/document_manager.hpp/cpp`: Document state management and symbol tracking
- `third_party/json.hpp`: nlohmann/json single-header library

### Adding New Features

1. Add handler method in `LSPServer` class
2. Add method routing in `handleMessage()`
3. Update server capabilities in `handleInitialize()`
4. Implement feature using `Document` and symbol table APIs

### Symbol Tracking

The symbol table builder uses a two-pass approach:

1. **First pass**: Map all identifier tokens to their positions
2. **Second pass**: Walk the AST and match symbols to tokens
   - First occurrence of a name = definition
   - Subsequent occurrences = references

This approach works around the AST storing symbol names as strings without position information.

## Performance

- Parsing is done on document open and change (full reparse)
- Symbol table is rebuilt on each parse
- Typical performance: <50ms for documents under 1000 lines

## Limitations

- Currently only supports full document sync (not incremental)
- Symbol tracking is per-document (no cross-file analysis)
- Hover info may not work for all positions (edge cases with token mapping)

## Future Enhancements

- [ ] Incremental document sync for better performance
- [ ] Cross-file symbol resolution (imports)
- [ ] Signature help for function calls
- [ ] Code actions and quick fixes
- [ ] Semantic tokens for better syntax highlighting
- [ ] Workspace symbols for cross-file search
- [ ] Document formatting

## See Also

- [LSP Specification](https://microsoft.github.io/language-server-protocol/)
- [IziLang LSP Documentation](../../docs/LSP_SERVER.md)
- [IziLang Main Repository](../../README.md)

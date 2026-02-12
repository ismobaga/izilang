# IziLang Language Server Protocol (LSP)

## Overview

The IziLang LSP server provides comprehensive IDE integration for IziLang with fully implemented features:

### ✅ Implemented Features
- **Diagnostics**: Real-time syntax errors and semantic warnings
- **Code Completion**: Context-aware suggestions (29+ keywords, all symbols)
- **Hover Information**: Type info, definition location, reference counts
- **Go to Definition**: Navigate from usage to declaration
- **Find References**: Locate all usages of a symbol
- **Rename Symbol**: Safe refactoring across the document
- **Document Symbols**: Code outline for quick navigation

### Future Enhancements
- Code formatting
- Signature help
- Code actions and quick fixes

## Architecture

```
┌─────────────────┐          JSON-RPC         ┌──────────────────┐
│   IDE / Editor  │ ◄──────────────────────► │  IziLang LSP    │
│  (VS Code, etc) │                           │     Server       │
└─────────────────┘                           └──────────────────┘
                                                      │
                                                      │ uses
                                                      ▼
                                              ┌──────────────────┐
                                              │  IziLang Parser  │
                                              │   & Analyzer     │
                                              └──────────────────┘
```

## Protocol Implementation

The LSP server implements the [Language Server Protocol](https://microsoft.github.io/language-server-protocol/) specification.

### Supported LSP Methods

#### Lifecycle
- `initialize`: Initialize the server with client capabilities
- `initialized`: Notification that client is ready
- `shutdown`: Shutdown the server
- `exit`: Exit the server process

#### Text Synchronization
- `textDocument/didOpen`: Document opened in editor
- `textDocument/didChange`: Document content changed
- `textDocument/didClose`: Document closed in editor
- `textDocument/didSave`: Document saved

#### Language Features
- `textDocument/completion`: Code completion suggestions
- `textDocument/hover`: Show hover information
- `textDocument/signatureHelp`: Function signature help
- `textDocument/definition`: Go to definition
- `textDocument/references`: Find all references
- `textDocument/documentSymbol`: Document outline
- `textDocument/formatting`: Format document
- `textDocument/rangeFormatting`: Format selection
- `textDocument/rename`: Rename symbol
- `textDocument/codeAction`: Code actions and quick fixes

#### Diagnostics
- `textDocument/publishDiagnostics`: Send errors and warnings to client

## Server Capabilities

```json
{
  "capabilities": {
    "textDocumentSync": {
      "openClose": true,
      "change": 2,
      "save": { "includeText": true }
    },
    "completionProvider": {
      "triggerCharacters": [".", "@"],
      "resolveProvider": false
    },
    "hoverProvider": true,
    "signatureHelpProvider": {
      "triggerCharacters": ["(", ","]
    },
    "definitionProvider": true,
    "referencesProvider": true,
    "documentSymbolProvider": true,
    "documentFormattingProvider": true,
    "documentRangeFormattingProvider": true,
    "renameProvider": true,
    "codeActionProvider": true
  }
}
```

## Implementation Details

### Parser Integration

The LSP server uses the IziLang parser to:
1. Parse source code into AST
2. Track symbol definitions and references
3. Perform type checking
4. Generate diagnostics

### Symbol Table

The server maintains a symbol table for:
- Variables (local and global)
- Functions
- Classes (future)
- Imports

### Completion Engine

Code completion provides:
- Keywords (`fn`, `var`, `if`, `while`, etc.)
- Built-in functions (`print`, `len`, etc.)
- User-defined functions
- Variables in scope
- Standard library members

### Diagnostics

The server reports:
- **Syntax errors**: Parse errors with position information
- **Semantic errors**: Type mismatches, undefined variables
- **Warnings**: Unused variables, deprecated features
- **Hints**: Code style suggestions

## Building the LSP Server

```bash
# Using premake5
cd tools/lsp
premake5 gmake2
make config=release

# Run the server
./bin/Release/izilang-lsp
```

## VS Code Extension

The VS Code extension connects to the LSP server:

```typescript
// extension.ts
import * as vscode from 'vscode';
import { LanguageClient } from 'vscode-languageclient/node';

export function activate(context: vscode.ExtensionContext) {
  const serverOptions = {
    command: 'izilang-lsp',
    args: ['--stdio']
  };

  const clientOptions = {
    documentSelector: [{ scheme: 'file', language: 'izilang' }]
  };

  const client = new LanguageClient(
    'izilang',
    'IziLang Language Server',
    serverOptions,
    clientOptions
  );

  client.start();
}
```

## Testing

The LSP server includes unit tests for:
- Protocol message handling
- Parser integration
- Completion generation
- Diagnostics accuracy

```bash
# Run tests
./bin/Release/izilang-lsp-tests
```

## Configuration

Users can configure the LSP server via `.vscode/settings.json`:

```json
{
  "izilang.lsp.trace.server": "verbose",
  "izilang.lsp.diagnostics.enable": true,
  "izilang.lsp.completion.enable": true,
  "izilang.format.indentSize": 4
}
```

## Future Enhancements

- **Code lens**: Show references count, run test annotations
- **Inlay hints**: Parameter names, type annotations
- **Semantic highlighting**: Advanced syntax coloring
- **Refactoring**: Extract method, inline variable
- **Debugging**: Debug adapter protocol integration
- **Performance**: Incremental parsing, caching

## Resources

- [LSP Specification](https://microsoft.github.io/language-server-protocol/)
- [VS Code Extension API](https://code.visualstudio.com/api)
- [Example LSP Servers](https://microsoft.github.io/language-server-protocol/implementors/servers/)

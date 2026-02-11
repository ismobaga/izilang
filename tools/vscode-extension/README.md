# IziLang VS Code Extension

Language support for IziLang in Visual Studio Code.

## Features

- **Syntax Highlighting**: Full syntax highlighting for IziLang code
- **Language Server**: Integration with IziLang LSP for advanced features
- **Code Completion**: Intelligent code completion
- **Diagnostics**: Real-time error and warning detection
- **Go to Definition**: Navigate to symbol definitions
- **Hover Information**: Show documentation on hover

## Installation

### From VSIX

1. Download the `.vsix` file from releases
2. Open VS Code
3. Go to Extensions view (Ctrl+Shift+X)
4. Click on "..." menu → "Install from VSIX..."
5. Select the downloaded file

### From Source

```bash
cd tools/vscode-extension
npm install
npm run compile
npm run package
code --install-extension izilang-0.1.0.vsix
```

## Requirements

The extension requires the IziLang LSP server to be installed and available in your PATH:

```bash
# Build LSP server
cd tools/lsp
premake5 gmake2
make config=release

# Install to PATH (Linux/macOS)
sudo ln -s $(pwd)/bin/Release/izilang-lsp /usr/local/bin/

# Or add to PATH in your shell config
export PATH="$PATH:/path/to/izilang/tools/lsp/bin/Release"
```

## Extension Settings

This extension contributes the following settings:

* `izilang.lsp.enable`: Enable/disable the IziLang language server
* `izilang.lsp.trace.server`: Set the verbosity of LSP communication logging

## Commands

* `IziLang: Restart Language Server`: Restart the LSP server

## File Association

The extension automatically activates for `.iz` files.

## Development

```bash
# Install dependencies
npm install

# Compile TypeScript
npm run compile

# Watch mode for development
npm run watch

# Package extension
npm run package
```

## Known Issues

- LSP server is currently a basic implementation
- Some advanced features are not yet available

## Release Notes

### 0.1.0

Initial release:
- Basic syntax highlighting
- LSP server integration skeleton
- File type association

## Contributing

Contributions are welcome! Please see the main IziLang repository for guidelines.

## License

MIT

# IziLang Tooling Summary

This document provides an overview of all tooling implemented for the IziLang project.

## Overview

The IziLang tooling ecosystem includes:

1. **CI/CD Pipeline**: Automated build, test, and release workflows
2. **Package Manager**: Dependency management and distribution
3. **LSP Server**: IDE integration for enhanced developer experience
4. **Web Playground**: Interactive browser-based code editor

## 1. CI/CD Pipeline

### Location
`.github/workflows/`

### Workflows

#### Build and Test (ci.yml)
- **Platforms**: Linux, macOS, Windows
- **Configurations**: Debug and Release
- **Features**:
  - Automated builds on push/PR
  - Test execution with Catch2
  - Artifact uploads for releases

#### Code Quality (code-quality.yml)
- **Tools**:
  - clang-format: Code formatting checks
  - clang-tidy: Static analysis
  - cppcheck: Additional code quality checks
- **Runs on**: Every push and PR

#### Release (release.yml)
- **Trigger**: Git tags (v*)
- **Outputs**: Platform-specific binaries
- **Platforms**: Linux, macOS, Windows
- **Format**: Compressed archives (.tar.gz, .zip)

#### Playground Deployment (deploy-playground.yml)
- **Trigger**: Changes to playground directory
- **Target**: GitHub Pages
- **Content**: Static web playground

### Usage

```bash
# Trigger CI on push
git push origin main

# Create a release
git tag v1.0.0
git push origin v1.0.0

# Workflows run automatically
```

### Documentation
See [GitHub Actions Documentation](https://docs.github.com/en/actions)

---

## 2. Package Manager

### Location
`tools/pkg/`

### Features

- Package initialization (`izi-pkg init`)
- Dependency installation (`izi-pkg install`)
- Package listing (`izi-pkg list`)
- Package search (`izi-pkg search`)
- Package updates (`izi-pkg update`)
- Package removal (`izi-pkg remove`)
- Publishing packages (`izi-pkg publish`)

### Package Format

```json
{
  "name": "my-package",
  "version": "1.0.0",
  "description": "A sample IziLang package",
  "main": "index.iz",
  "dependencies": {
    "std-math": "^1.0.0"
  }
}
```

### Installation

```bash
# Add to PATH
export PATH="$PATH:/path/to/izilang/tools/pkg"

# Or create symlink
sudo ln -s /path/to/izilang/tools/pkg/izi-pkg /usr/local/bin/
```

### Usage

```bash
# Initialize new package
izi-pkg init

# Install dependencies
izi-pkg install

# Install specific package
izi-pkg install std-math

# List installed packages
izi-pkg list
```

### Documentation
- [Package Manager Spec](docs/PACKAGE_MANAGER.md)
- [Package Tool README](tools/pkg/README.md)

---

## 3. LSP Server

### Location
`tools/lsp/`

### Features

- **Text Synchronization**: Track document changes
- **Diagnostics**: Error and warning reporting
- **Code Completion**: Context-aware suggestions
- **Hover Information**: Symbol documentation
- **Go to Definition**: Navigate to declarations
- **Find References**: Locate symbol usage
- **Document Symbols**: Code outline
- **Formatting**: Code style enforcement

### Building

```bash
cd tools/lsp
premake5 gmake2
make config=release
```

### VS Code Extension

Location: `tools/vscode-extension/`

Features:
- Syntax highlighting for .iz files
- LSP client integration
- Command palette commands
- Configuration settings

### Installation

```bash
# Build LSP server
cd tools/lsp
make config=release

# Install VS Code extension
cd ../vscode-extension
npm install
npm run compile
npm run package
code --install-extension izilang-0.1.0.vsix
```

### Usage

1. Open VS Code
2. Open a .iz file
3. LSP features activate automatically
4. Use Ctrl+Space for completion, hover for docs, etc.

### Documentation
- [LSP Server Spec](docs/LSP_SERVER.md)
- [LSP README](tools/lsp/README.md)
- [VS Code Extension README](tools/vscode-extension/README.md)

---

## 4. Web Playground

### Location
`playground/`

### Features

- **Code Editor**: Write IziLang code in browser
- **Example Library**: Pre-built code examples
- **Code Sharing**: Share code via URL
- **Instant Execution**: Run code (with WASM)
- **Dark Theme**: Developer-friendly UI
- **Keyboard Shortcuts**: Ctrl+Enter to run

### Running Locally

```bash
cd playground/public
python3 -m http.server 8000
# Visit http://localhost:8000
```

### Deployment

The playground is automatically deployed to GitHub Pages on push to main branch.

Access at: `https://yourusername.github.io/izilang/`

### WebAssembly Compilation

To enable code execution:

```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# Compile IziLang to WASM
emcc src/**/*.cpp \
    -I src \
    -o playground/public/izilang.js \
    -s WASM=1 \
    -O3 \
    --bind
```

### Documentation
- [Playground Overview](playground/README.md)
- [Web Playground Spec](docs/WEB_PLAYGROUND.md)

---

## Quick Reference

### Directory Structure

```
izilang/
├── .github/
│   └── workflows/          # CI/CD workflows
│       ├── ci.yml
│       ├── code-quality.yml
│       ├── release.yml
│       └── deploy-playground.yml
├── docs/
│   ├── PACKAGE_MANAGER.md  # Package manager spec
│   ├── LSP_SERVER.md       # LSP protocol spec
│   └── WEB_PLAYGROUND.md   # Playground architecture
├── tools/
│   ├── pkg/                # Package manager
│   │   ├── izi-pkg         # CLI tool
│   │   └── README.md
│   ├── lsp/                # LSP server
│   │   ├── src/
│   │   ├── premake5.lua
│   │   └── README.md
│   └── vscode-extension/   # VS Code extension
│       ├── src/
│       ├── syntaxes/
│       ├── package.json
│       └── README.md
└── playground/             # Web playground
    ├── public/
    │   ├── index.html
    │   └── playground.js
    └── README.md
```

### Commands Cheat Sheet

```bash
# CI/CD
git push                    # Trigger CI
git tag v1.0.0 && git push origin v1.0.0  # Create release

# Package Manager
izi-pkg init               # Initialize package
izi-pkg install            # Install dependencies
izi-pkg list               # List packages

# LSP Server
cd tools/lsp && make       # Build LSP
izilang-lsp --stdio        # Run LSP server

# VS Code Extension
cd tools/vscode-extension
npm run compile            # Build extension
npm run package            # Create VSIX

# Playground
cd playground/public
python3 -m http.server     # Run locally
```

---

## Implementation Status

### Completed ✓

- [x] CI/CD pipeline with multi-platform support
- [x] Automated testing workflow
- [x] Code quality checks
- [x] Release automation
- [x] Package manager specification
- [x] Package manager CLI tool
- [x] LSP server skeleton
- [x] VS Code extension with syntax highlighting
- [x] Web playground UI
- [x] Playground deployment workflow
- [x] Comprehensive documentation

### In Progress / Future Work

- [ ] Full package registry implementation
- [ ] LSP server with full language features
- [ ] WebAssembly compilation for playground
- [ ] Monaco editor integration
- [ ] Package signing and verification
- [ ] LSP server performance optimization
- [ ] Mobile-responsive playground
- [ ] Additional IDE extensions (Vim, Emacs, IntelliJ)

---

## Getting Started

### For Contributors

1. **Set up CI/CD**: Already configured, just push code
2. **Use Package Manager**: `izi-pkg init` in your project
3. **Install VS Code Extension**: Follow extension README
4. **Try Playground**: Visit the GitHub Pages URL

### For Users

1. **Download Releases**: Get binaries from GitHub Releases
2. **Install Extension**: Get IziLang extension from VS Code marketplace (future)
3. **Use Playground**: Try IziLang online at the playground URL

---

## Resources

### Internal Documentation
- [Package Manager Spec](docs/PACKAGE_MANAGER.md)
- [LSP Server Spec](docs/LSP_SERVER.md)
- [Web Playground Spec](docs/WEB_PLAYGROUND.md)
- [Main README](README.md)
- [Analysis](ANALYSIS.md)
- [Next Steps](NEXT_STEPS.md)

### External Resources
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Language Server Protocol](https://microsoft.github.io/language-server-protocol/)
- [VS Code Extension API](https://code.visualstudio.com/api)
- [Emscripten](https://emscripten.org/)
- [WebAssembly](https://webassembly.org/)

---

## Support

For issues or questions:
- Open an issue on GitHub
- Check documentation in `docs/`
- Review tool-specific READMEs

## Contributing

Contributions to tooling are welcome! Please:
1. Read the relevant documentation
2. Follow existing code style
3. Add tests where applicable
4. Update documentation
5. Submit a pull request

---

**Last Updated**: 2026-02-11

This tooling infrastructure provides a solid foundation for IziLang development and distribution. All components are designed to be extensible and maintainable.

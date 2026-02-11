# IziLang Package Manager

A simple command-line package manager for IziLang projects.

## Installation

The package manager is available in the `tools/pkg` directory. You can add it to your PATH:

```bash
# Add to PATH in your .bashrc or .zshrc
export PATH="$PATH:/path/to/izilang/tools/pkg"
```

Or create a symlink:

```bash
sudo ln -s /path/to/izilang/tools/pkg/izi-pkg /usr/local/bin/izi-pkg
```

## Quick Start

```bash
# Initialize a new package
izi-pkg init

# Install dependencies
izi-pkg install

# List installed packages
izi-pkg list
```

## Documentation

See [PACKAGE_MANAGER.md](../../docs/PACKAGE_MANAGER.md) for full documentation.

## Current Status

This is a prototype implementation. The following features are planned:

- ✅ CLI interface and commands
- ✅ Package manifest format (package.json)
- ⬜ Package installation from registry
- ⬜ Package publishing
- ⬜ Dependency resolution
- ⬜ Version management
- ⬜ Git-based package installation

## Development

The package manager is currently a bash script for rapid prototyping. Future versions may be implemented in C++ or as part of the main IziLang binary.

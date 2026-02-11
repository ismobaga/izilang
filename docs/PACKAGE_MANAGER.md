# IziLang Package Manager Specification

## Overview

The IziLang package manager (`izi pkg`) provides dependency management and distribution for IziLang projects.

## Package Manifest Format

Packages are defined using a `package.json` file in the project root:

```json
{
  "name": "my-package",
  "version": "1.0.0",
  "description": "A sample IziLang package",
  "main": "index.iz",
  "author": "Your Name",
  "license": "MIT",
  "dependencies": {
    "std-math": "^1.0.0",
    "std-io": "^2.1.0"
  },
  "devDependencies": {
    "test-framework": "^0.5.0"
  },
  "scripts": {
    "test": "izi test",
    "build": "izi build"
  },
  "repository": {
    "type": "git",
    "url": "https://github.com/username/my-package"
  }
}
```

## Package Structure

```
my-package/
├── package.json          # Package manifest
├── index.iz             # Main entry point
├── src/                 # Source files
│   ├── module1.iz
│   └── module2.iz
├── tests/               # Test files
│   └── test_main.iz
└── README.md            # Documentation
```

## Package Registry

Packages can be published to and installed from:
- **Official Registry**: `https://packages.izilang.org` (future)
- **Git Repositories**: Direct installation from GitHub, GitLab, etc.
- **Local Packages**: Development from local filesystem

## Commands

### `izi pkg init`
Initialize a new package with interactive prompts:
```bash
izi pkg init
```

Creates a `package.json` in the current directory.

### `izi pkg install [package]`
Install dependencies:
```bash
# Install all dependencies from package.json
izi pkg install

# Install a specific package
izi pkg install std-math

# Install from git
izi pkg install git+https://github.com/user/package.git

# Install specific version
izi pkg install std-math@1.2.0
```

### `izi pkg publish`
Publish package to registry:
```bash
izi pkg publish
```

### `izi pkg search [query]`
Search for packages:
```bash
izi pkg search math
```

### `izi pkg list`
List installed packages:
```bash
izi pkg list
```

### `izi pkg update [package]`
Update packages:
```bash
# Update all packages
izi pkg update

# Update specific package
izi pkg update std-math
```

### `izi pkg remove [package]`
Remove a package:
```bash
izi pkg remove std-math
```

## Version Resolution

The package manager uses semantic versioning (semver):
- `^1.2.3`: Compatible with 1.x.x (>= 1.2.3, < 2.0.0)
- `~1.2.3`: Approximately 1.2.x (>= 1.2.3, < 1.3.0)
- `1.2.3`: Exact version
- `*` or `latest`: Latest version

## Installation Location

Packages are installed to:
- Project-local: `./izi_modules/` (default)
- Global: `~/.izi/packages/` (with `-g` flag)

## Lock File

A `package-lock.json` file ensures reproducible installations:
```json
{
  "lockfileVersion": 1,
  "dependencies": {
    "std-math": {
      "version": "1.0.0",
      "resolved": "https://packages.izilang.org/std-math/-/std-math-1.0.0.tgz",
      "integrity": "sha512-..."
    }
  }
}
```

## Import Resolution

When code imports a package:
```izilang
import "std-math";
```

The import resolver checks:
1. `./izi_modules/std-math/` (project dependencies)
2. `~/.izi/packages/std-math/` (global packages)
3. Built-in standard library

## Publishing Workflow

1. Create account on registry (future)
2. Update `package.json` with version and details
3. Run `izi pkg publish`
4. Package is validated and uploaded
5. Users can now `izi pkg install your-package`

## Private Packages

Support for private registries and authentication:
```bash
# Configure registry
izi pkg config set registry https://private-registry.company.com

# Login
izi pkg login
```

## Future Enhancements

- Binary package support (native extensions)
- Package verification and signatures
- Automated testing in CI/CD
- Package analytics and download stats
- Scoped packages (@username/package)

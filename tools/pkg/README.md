# IziLang Package Manager (`izi-pkg`)

A project-local, TOML-manifest-based dependency manager for IziLang.
Designed from the ground up for IziLang — it does **not** replicate npm/yarn/pnpm
concepts or workflows.

## Installation

Add the `tools/pkg` directory to your `PATH`:

```bash
export PATH="$PATH:/path/to/izilang/tools/pkg"
```

Or create a symlink:

```bash
sudo ln -s /path/to/izilang/tools/pkg/izi-pkg /usr/local/bin/izi-pkg
```

## Quick Start

```bash
# Scaffold a new project (creates izi.toml)
izi-pkg new

# Add a dependency
izi-pkg add std-math@1.0.0

# Fetch all dependencies into libs/
izi-pkg sync

# Show installed packages
izi-pkg show
```

## Key Differences from npm/yarn/pnpm

| Concept          | npm / yarn / pnpm         | izi-pkg                        |
|------------------|--------------------------|-------------------------------|
| Manifest         | `package.json` (JSON)    | `izi.toml` (TOML)             |
| Lock file        | `package-lock.json`      | `izi.lock`                    |
| Package dir      | `node_modules/`          | `libs/`                       |
| Version ranges   | `^1.2`, `~1.2`, `*`      | Exact versions only           |
| Global installs  | `npm install -g`         | Not supported                 |
| Lifecycle hooks  | `scripts` field          | Use a Makefile instead        |

## Documentation

See [../../docs/PACKAGE_MANAGER.md](../../docs/PACKAGE_MANAGER.md) for the full specification,
including the comparison table, manifest schema, lock-file format, and implementation plan.

## Current Status

This is a prototype implementation following the redesigned specification.

| Feature                        | Status |
|-------------------------------|--------|
| `izi-pkg new` (scaffold)       | ✅ Implemented |
| `izi-pkg add` (add dependency) | ✅ Implemented (manifest edit only) |
| `izi-pkg drop` (remove dep)    | ✅ Implemented |
| `izi-pkg show` (list libs/)    | ✅ Implemented |
| `izi-pkg sync` (fetch deps)    | ⬜ Phase 2/3 — registry not yet live |
| `izi-pkg find` (search hub)    | ⬜ Phase 3 — hub not yet live |
| `izi-pkg ship` (publish)       | ⬜ Phase 3 — hub not yet live |
| `izi-pkg upgrade`              | ⬜ Phase 4 |

## Development

The package manager is currently a Bash script for rapid prototyping.
A future version will be implemented in C++ and integrated into the main `izi` binary.

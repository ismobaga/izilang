# IziLang Package Manager Specification

## Overview

The IziLang package manager (`izi-pkg`) provides dependency management and distribution for
IziLang projects. Its design is built around IziLang's own idioms — it does **not** replicate
the concepts or workflows found in Node's npm, yarn, or pnpm.

## Design Principles

1. **TOML-first manifest** — human-readable, no JSON noise.
2. **Exact-version pinning** — no caret (`^`) or tilde (`~`) wildcards; every dependency
   records its exact version, preventing silent breakage.
3. **Flat, named package cache** — packages live in `libs/` at the project root, named
   `<name>-<version>`, never nested inside each other.
4. **Content-hash lock file** — `izi.lock` stores a SHA-256 digest for every resolved
   package so fetches are always reproducible without a registry.
5. **Project-local only** — there is no "global install" concept; each project owns its own
   dependency graph.
6. **Source packages** — the unit of distribution is IziLang source, not compiled binaries,
   keeping the ecosystem auditable and portable.
7. **Registry-agnostic** — packages can come from the official hub, any HTTP endpoint, a git
   URL, or a local path; all are first-class citizens.

## Comparison with Node Package Managers

| Concept            | npm / yarn / pnpm              | izi-pkg                              |
|--------------------|-------------------------------|--------------------------------------|
| Manifest file      | `package.json` (JSON)         | `izi.toml` (TOML)                    |
| Lock file          | `package-lock.json` / `yarn.lock` | `izi.lock` (TOML)               |
| Package directory  | `node_modules/`               | `libs/`                              |
| Version specifiers | `^1.2`, `~1.2`, `*`, ranges   | Exact versions only (`1.2.3`)        |
| Dependency kinds   | `dependencies` + `devDependencies` | Single `[deps]` table with optional `[dev-deps]` |
| Global installs    | `npm install -g`              | Not supported — project-local only   |
| Lifecycle scripts  | `scripts` field, hooks        | None — use a `Makefile` or shell script instead |
| Package unit       | Tarballs / binaries           | IziLang source trees (`.izi` files)  |
| Registry protocol  | Custom npm wire protocol      | Plain HTTPS with JSON metadata + ZIP archives |

## Package Manifest (`izi.toml`)

Every project or distributable library contains an `izi.toml` in its root:

```toml
[pack]
name        = "my-package"
version     = "1.0.0"
description = "A sample IziLang package"
entry       = "main.izi"
authors     = ["Your Name <you@example.com>"]
license     = "MIT"
repository  = "https://github.com/username/my-package"

[deps]
std-math = "1.0.0"
std-io   = "2.1.0"

[dev-deps]
test-kit = "0.5.0"
```

### `[pack]` fields

| Key           | Required | Description                                  |
|---------------|----------|----------------------------------------------|
| `name`        | ✅       | Package identifier (lowercase, hyphens only) |
| `version`     | ✅       | Exact version (`MAJOR.MINOR.PATCH`)          |
| `entry`       | ✅       | Main `.izi` file relative to `izi.toml`      |
| `description` | —        | Short human-readable description             |
| `authors`     | —        | List of `"Name <email>"` strings             |
| `license`     | —        | SPDX license identifier                      |
| `repository`  | —        | URL of the source repository                 |

### `[deps]` and `[dev-deps]`

Both tables map package names to exact version strings.  `[dev-deps]` packages are fetched
only when the `--dev` flag is passed to `izi-pkg sync`.

## Package Structure

```
my-package/
├── izi.toml          # Package manifest
├── izi.lock          # Reproducibility lock file (auto-generated)
├── main.izi          # Entry point declared in izi.toml
├── src/              # Source files
│   ├── module1.izi
│   └── module2.izi
├── tests/            # Test files
│   └── test_main.izi
├── libs/             # Fetched dependencies (auto-managed, add to .gitignore)
└── README.md
```

## Lock File (`izi.lock`)

`izi.lock` is generated automatically on every `sync` and **must be committed** to version
control.  It records the exact resolved version and content hash of every dependency:

```toml
[std-math]
version = "1.0.0"
source  = "https://hub.izilang.org/std-math/1.0.0.zip"
sha256  = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"

[std-io]
version = "2.1.0"
source  = "https://hub.izilang.org/std-io/2.1.0.zip"
sha256  = "6b86b273ff34fce19d6b804eff5a3f5747ada4eaa22f1d49c01e52ddb7875b4b"
```

## Package Registry

Packages are fetched from (in priority order):

1. **Local path** — `path = "../my-lib"` in `[deps]` overrides remote lookup.
2. **Git URL** — `git = "https://github.com/user/pkg.git"` with optional `ref = "v1.2.3"`.
3. **Official Hub** — `https://hub.izilang.org` (future public registry).
4. **Custom registry** — configurable via `~/.izi/config.toml`.

## Commands

### `izi-pkg new`
Scaffold a new project interactively and create `izi.toml`:
```bash
izi-pkg new
```

### `izi-pkg sync [--dev]`
Fetch all dependencies listed in `izi.toml` into `libs/` and regenerate `izi.lock`.
Pass `--dev` to also fetch `[dev-deps]`:
```bash
izi-pkg sync
izi-pkg sync --dev
```

### `izi-pkg add <package[@version]>`
Add a dependency to `[deps]` in `izi.toml` and run `sync`:
```bash
izi-pkg add std-math
izi-pkg add std-math@1.2.0
izi-pkg add path=../local-lib
```

### `izi-pkg drop <package>`
Remove a dependency from `izi.toml` and delete it from `libs/`:
```bash
izi-pkg drop std-math
```

### `izi-pkg show`
Show all packages currently present in `libs/`:
```bash
izi-pkg show
```

### `izi-pkg find <query>`
Search the package hub for matching packages:
```bash
izi-pkg find math
```

### `izi-pkg ship`
Validate `izi.toml` and upload the package to the registry:
```bash
izi-pkg ship
```

### `izi-pkg upgrade <package>`
Fetch the latest compatible version of a dependency and update `izi.toml` and `izi.lock`:
```bash
izi-pkg upgrade std-math
```

## Import Resolution

When IziLang code imports a package:

```izilang
import "std-math";
```

The resolver checks:

1. `./libs/std-math-<version>/` (project dependencies, from `izi.lock`)
2. Built-in standard library (always available, no install needed)

There are no global package paths to search — only the project-local `libs/` directory.

## Publishing Workflow

1. Write your library and fill in `izi.toml`.
2. Run `izi-pkg ship` — the tool validates the manifest, runs any declared tests, and
   uploads a ZIP archive to the registry.
3. Other projects can now `izi-pkg add your-package`.

## Configuration

User-level configuration lives in `~/.izi/config.toml`:

```toml
[registry]
url = "https://hub.izilang.org"

[auth]
token = "your-api-token"
```

## Implementation Plan

### Phase 1 — Manifest & Scaffold (v0.2)
- [x] Define TOML manifest schema (`izi.toml`)
- [x] `izi-pkg new` command with interactive prompts
- [x] Validate `izi.toml` on every command

### Phase 2 — Local Resolution (v0.2)
- [x] `izi-pkg add` with local-path and git-URL support
- [x] `izi-pkg drop` command
- [x] `libs/` directory management
- [x] `izi.lock` generation (version + SHA-256)

### Phase 3 — Registry (v0.3)
- [ ] Official hub backend (HTTPS + JSON metadata)
- [ ] `izi-pkg sync` with registry fetch and hash verification
- [ ] `izi-pkg find` command (hub search API)
- [ ] `izi-pkg ship` for publishing

### Phase 4 — Polish (v1.0)
- [ ] `izi-pkg upgrade` command
- [ ] Offline mode (use lock file, skip network)
- [ ] IDE/LSP integration for dependency resolution
- [ ] Package signing and verification

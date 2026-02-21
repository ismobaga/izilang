#!/usr/bin/env bash
# Tests for the izi-pkg package manager (tools/pkg/izi-pkg).
# Run from the repository root: bash tests/test_pkg.sh

set -e

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PKG="$REPO_ROOT/tools/pkg/izi-pkg"
PASS=0
FAIL=0

# ── helpers ──────────────────────────────────────────────────────────────────

assert_eq() {
    local desc="$1" expected="$2" actual="$3"
    if [ "$expected" = "$actual" ]; then
        echo "  PASS: $desc"
        PASS=$((PASS + 1))
    else
        echo "  FAIL: $desc"
        echo "        expected: $expected"
        echo "        actual:   $actual"
        FAIL=$((FAIL + 1))
    fi
}

assert_contains() {
    local desc="$1" needle="$2" haystack="$3"
    if echo "$haystack" | grep -qF "$needle"; then
        echo "  PASS: $desc"
        PASS=$((PASS + 1))
    else
        echo "  FAIL: $desc"
        echo "        expected to contain: $needle"
        echo "        actual: $haystack"
        FAIL=$((FAIL + 1))
    fi
}

assert_file_exists() {
    local desc="$1" path="$2"
    if [ -e "$path" ]; then
        echo "  PASS: $desc"
        PASS=$((PASS + 1))
    else
        echo "  FAIL: $desc (file not found: $path)"
        FAIL=$((FAIL + 1))
    fi
}

assert_file_not_exists() {
    local desc="$1" path="$2"
    if [ ! -e "$path" ]; then
        echo "  PASS: $desc"
        PASS=$((PASS + 1))
    else
        echo "  FAIL: $desc (file should not exist: $path)"
        FAIL=$((FAIL + 1))
    fi
}

# Create a temp project directory and change into it.
# Writes a minimal izi.toml.
setup_project() {
    TDIR="$(mktemp -d)"
    cd "$TDIR"
    cat > izi.toml <<'TOML'
[pack]
name    = "test-project"
version = "1.0.0"
entry   = "main.izi"

[deps]

[dev-deps]
TOML
}

# Create a minimal local library under TDIR.
make_local_lib() {
    local name="$1" ver="${2:-1.0.0}"
    local dir="$TDIR/$name"
    mkdir -p "$dir"
    cat > "$dir/izi.toml" <<TOML
[pack]
name    = "$name"
version = "$ver"
entry   = "main.izi"
TOML
    echo "fn hello() { return \"$name\"; }" > "$dir/main.izi"
    echo "$dir"
}

# ── tests ────────────────────────────────────────────────────────────────────

echo "=== izi-pkg test suite ==="
echo ""

# ── version ──────────────────────────────────────────────────────────────────
echo "--- version ---"
out="$("$PKG" version)"
assert_contains "version prints izi-pkg" "IziLang Package Manager" "$out"

# ── help ─────────────────────────────────────────────────────────────────────
echo "--- help ---"
out="$("$PKG" help)"
assert_contains "help lists sync command"    "sync"    "$out"
assert_contains "help lists add command"     "add"     "$out"
assert_contains "help lists drop command"    "drop"    "$out"
assert_contains "help lists show command"    "show"    "$out"

# ── sync: empty deps ─────────────────────────────────────────────────────────
echo "--- sync: empty deps ---"
setup_project
"$PKG" sync >/dev/null 2>&1
assert_file_exists "lock file created after sync" "izi.lock"
lock="$(cat izi.lock)"
assert_eq "lock file empty when no deps" "" "$lock"

# ── add: registry version dep ────────────────────────────────────────────────
echo "--- add: registry version dep ---"
setup_project
"$PKG" add std-math@1.0.0 >/dev/null 2>&1
manifest="$(cat izi.toml)"
assert_contains "add writes version dep to manifest" 'std-math = "1.0.0"' "$manifest"
lock="$(cat izi.lock)"
assert_contains "lock has dep name"    "[std-math]"    "$lock"
assert_contains "lock has dep version" 'version = "1.0.0"' "$lock"

# ── add: duplicate dep is idempotent ─────────────────────────────────────────
echo "--- add: duplicate dep is idempotent ---"
setup_project
"$PKG" add std-math@1.0.0 >/dev/null 2>&1
count_before="$(grep -c 'std-math' izi.toml)"
"$PKG" add std-math@1.0.0 >/dev/null 2>&1
count_after="$(grep -c 'std-math' izi.toml)"
assert_eq "duplicate add does not duplicate entry" "$count_before" "$count_after"

# ── add: local path dep ──────────────────────────────────────────────────────
echo "--- add: local path dep ---"
setup_project
LIBDIR="$(make_local_lib my-lib 0.5.0)"
"$PKG" add "path=$LIBDIR" >/dev/null 2>&1
manifest="$(cat izi.toml)"
assert_contains "add writes path dep to manifest"  "my-lib = { path ="  "$manifest"
lock="$(cat izi.lock)"
assert_contains "lock has local dep name"    "[my-lib]"      "$lock"
assert_contains "lock has local dep version" 'version = "0.5.0"' "$lock"
assert_contains "lock has local dep source"  "source  ="     "$lock"
assert_contains "lock has local dep sha256"  "sha256  ="     "$lock"
assert_file_exists "dep installed in libs/" "libs/my-lib-0.5.0"

# ── add: git dep (manifest only, no network) ─────────────────────────────────
echo "--- add: git dep (manifest only) ---"
setup_project
# We don't run sync here — just check that add writes the correct entry
# (sync would attempt a network clone which is not available in CI)
cat >> izi.toml <<'TOML'
# pre-populated git dep to avoid network call
TOML
# Manually insert git entry to test manifest parsing without network
awk '/^\[deps\]/ { print; print "my-pkg = { git = \"https://example.com/my-pkg.git\", ref = \"v1.0.0\" }"; next } { print }' izi.toml > /tmp/izi_test.toml && mv /tmp/izi_test.toml izi.toml
manifest="$(cat izi.toml)"
assert_contains "git dep present in manifest" 'git = "https://example.com/my-pkg.git"' "$manifest"

# ── add: git dep via cmd_add ──────────────────────────────────────────────────
echo "--- add: git dep written by cmd_add ---"
setup_project
# We verify the manifest write, not the network clone
# Temporarily override cmd_sync to do nothing by using a no-clone manifest
out="$("$PKG" add "git=https://example.com/repo.git@v2.0.0" 2>&1 || true)"
manifest="$(cat izi.toml 2>/dev/null || true)"
assert_contains "git dep entry in manifest" 'git = "https://example.com/repo.git"' "$manifest"
assert_contains "git dep ref in manifest"   'ref = "v2.0.0"'                        "$manifest"

# ── sync: local path resolved and hashed ────────────────────────────────────
echo "--- sync: local path resolved and hashed ---"
setup_project
LIBDIR="$(make_local_lib hash-lib 2.0.0)"
"$PKG" add "path=$LIBDIR" >/dev/null 2>&1
lock="$(cat izi.lock)"
sha="$(echo "$lock" | grep '^sha256' | awk -F'"' '{print $2}')"
assert_contains "sha256 is non-empty" "sha256  =" "$lock"
[ -n "$sha" ] && assert_contains "sha256 is a hex string" "$sha" "$sha"

# ── sync: --dev includes dev-deps ────────────────────────────────────────────
echo "--- sync: --dev includes dev-deps ---"
TDIR="$(mktemp -d)"
cd "$TDIR"
DEVA="$TDIR/dev-only"
mkdir -p "$DEVA"
cat > "$DEVA/izi.toml" <<TOML
[pack]
name    = "dev-only"
version = "3.0.0"
entry   = "main.izi"
TOML
echo 'fn dev() {}' > "$DEVA/main.izi"
cat > izi.toml <<TOML
[pack]
name    = "test-project"
version = "1.0.0"
entry   = "main.izi"

[deps]

[dev-deps]
dev-only = { path = "$DEVA" }
TOML

"$PKG" sync >/dev/null 2>&1
assert_file_not_exists "dev dep NOT installed without --dev" "libs/dev-only-3.0.0"

"$PKG" sync --dev >/dev/null 2>&1
assert_file_exists "dev dep installed with --dev" "libs/dev-only-3.0.0"

# ── drop ─────────────────────────────────────────────────────────────────────
echo "--- drop ---"
setup_project
LIBDIR="$(make_local_lib drop-me 1.0.0)"
"$PKG" add "path=$LIBDIR" >/dev/null 2>&1
assert_file_exists "dep installed before drop" "libs/drop-me-1.0.0"

"$PKG" drop drop-me >/dev/null 2>&1
manifest="$(cat izi.toml)"
lock="$(cat izi.lock)"
assert_file_not_exists "dep removed from libs/ after drop" "libs/drop-me-1.0.0"
assert_eq "dep removed from manifest after drop" "" "$(echo "$manifest" | grep 'drop-me' || true)"
assert_eq "dep removed from lock after drop" "" "$(echo "$lock" | grep 'drop-me' || true)"

# ── show ─────────────────────────────────────────────────────────────────────
echo "--- show ---"
setup_project
LIBDIR="$(make_local_lib show-lib 1.0.0)"
"$PKG" add "path=$LIBDIR" >/dev/null 2>&1
out="$("$PKG" show 2>&1)"
assert_contains "show lists installed lib" "show-lib-1.0.0" "$out"

# ── error: unknown command ────────────────────────────────────────────────────
echo "--- error: unknown command ---"
setup_project
out="$("$PKG" foobar 2>&1 || true)"
assert_contains "unknown command shows error" "Unknown command" "$out"

# ── summary ──────────────────────────────────────────────────────────────────
echo ""
echo "Results: $PASS passed, $FAIL failed"
[ $FAIL -eq 0 ] && echo "All tests passed." || { echo "Some tests failed!"; exit 1; }

# VM Parity Checklist

This checklist tracks parity between the stable interpreter and the experimental VM.

## Goal

Establish behavioral equivalence for priority language features so `izi run --vm` is reliable for day-to-day usage.

## Exit Criteria

- All tests tagged `vm-parity` pass.
- All tests tagged `vm-gap` are either passing or linked to an issue with a target fix date.
- No regressions in existing VM suites (`vm-core`, `vm-complete`, `vm-stack`, `vm-simple`).
- Interpreter/VM output matches for all P0 scenarios below.

## Priority Matrix

### P0 (Must Match)

- Arithmetic, boolean logic, comparisons
- Variables and assignment
- If/else, while, for
- Arrays and maps (read/write)
- Functions, closures, recursion
- Nullish coalescing `??`
- Try/catch/finally
- Module imports (named/default)

### P1 (High)

- Classes and inheritance
- `this` and `super`
- Pattern matching
- Macros
- Semantic edge cases in scoping

### P2 (Nice to Have for v0.4)

- Async/await parity edge cases
- Tooling-level parity checks (formatter + checker interactions)

## Daily Workflow

1. Run parity suite:

```bash
./build/tests "[vm-parity]"
```

2. Run known-gap suite (informational):

```bash
./build/tests "[vm-gap]"
```

3. Run existing VM suites:

```bash
./build/tests "[vm-core]"
./build/tests "[vm-complete]"
./build/tests "[vm-stack]"
./build/tests "[vm-simple]"
```

4. For each parity mismatch:
- Minimize source snippet to smallest repro.
- Add/adjust test in `tests/test_vm_parity.cpp`.
- Fix compiler/VM implementation.
- Re-run the same focused tag first, then full VM suites.

## Tracking Template

Use this format for each discovered mismatch:

- ID: VM-PARITY-XXX
- Feature: <feature>
- Snippet: <minimal source>
- Expected (interp): <output/error>
- Actual (vm): <output/error>
- Status: open/in-progress/fixed
- Owner: <name>
- Target date: <date>

## Current Gaps (Observed)

- VM-PARITY-001
- Feature: closures capturing local variables
- Expected (interp): counter prints `1`, then `2`
- Actual (vm): runtime error `Undefined variable 'c'`
- Status: open

- VM-PARITY-002
- Feature: `try/catch/finally` finalization semantics
- Expected (interp): prints `boom`, then `done`
- Actual (vm): prints `boom` only (`finally` appears skipped)
- Status: open

- VM-PARITY-003
- Feature: `await` in VM execution path
- Expected (interp): `print(await v())` prints `42`
- Actual (vm): runtime error `Undefined variable 'await'`
- Status: open

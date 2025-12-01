# IziLang AI Coding Instructions

## Project Overview
IziLang is a programming language interpreter implemented in C++20. The project uses the Visitor pattern for AST traversal and `std::variant` for runtime values.

## Architecture

### Core Components
- **AST Layer** (`src/ast/`): Expression and statement nodes with visitor interfaces
- **Common Types** (`src/common/`): Token definitions and runtime value representations
- **Visitor Pattern**: All AST nodes implement `accept()` methods for double-dispatch

### Key Design Patterns

**Visitor Pattern for AST**:
- All expression types inherit from `Expr` and implement `accept(ExprVisitor&)`
- `ExprVisitor` defines pure virtual `visit()` methods for each expression type
- Example: `BinaryExpr`, `UnaryExpr`, `LiteralExpr` all call `visitor.visit(*this)`
- Same pattern applies to statements with `StmtVisitor`

**Runtime Values via std::variant**:
- `Value` type in `src/common/value.h` uses `std::variant<Nil, bool, double, std::string>`
- Use `std::holds_alternative<T>()` and `std::get<T>()` for type checking and extraction
- Helper functions: `isTruthy()`, `asNumber()`, `printValue()`

**Smart Pointer Ownership**:
- AST nodes use `std::unique_ptr<Expr>` (aliased as `ExprPtr`)
- Constructor parameters take `ExprPtr` by value and use `std::move()`
- Example: `BinaryExpr(ExprPtr l, Token o, ExprPtr r) : left(std::move(l)), ...`

## Language Features

**Supported Token Types** (see `src/common/token.h`):
- Operators: `+`, `-`, `*`, `/`, `!`, `==`, `!=`, `<`, `>`, `<=`, `>=`
- Keywords: `fn`, `var`, `if`, `else`, `while`, `for`, `return`, `break`, `continue`, `class`
- Arrow syntax: `->` token for potential function syntax

**Expression Types** (`src/ast/expr.h`):
- `LiteralExpr`: Direct value storage
- `VariableExpr`: Has both `name` and `value` fields
- `BinaryExpr`: Left operand, operator token, right operand
- `UnaryExpr`: Operator token and right operand
- `GroupingExpr`: Parenthesized expression wrapper

**Forward Declarations**: `src/ast/visitor.h` declares statement types (`ExprStmt`, `BlockStmt`, `VarStmt`, `WhileStmt`, `ReturnStmt`, `FunctionStmt`, `IfStmt`) but implementations not yet in codebase.

## Build System

**Premake5 Configuration**:
- Project name: "izi" (ConsoleApp)
- C++20 standard required (`cppdialect "c++20"`)
- Configurations: Debug (with symbols) and Release (optimized)
- Linux requires linking: `m`, `dl`, `pthread`

**Build Commands**:
```bash
premake5 gmake2           # Generate makefiles
make config=debug         # Build debug version
make config=release       # Build release version
./bin/Debug-linux-x86_64/izi    # Run debug build
```

## Coding Conventions

- **Namespace**: All code in `namespace izi`
- **Headers**: Use `#pragma once` for include guards
- **Include Paths**: Relative from `src/` (e.g., `#include "common/token.h"`)
- **Naming**: CamelCase for types, lowercase for variables/functions
- **Token Storage**: `Token` struct uses `std::string_view` for zero-copy lexeme storage
- **Virtual Methods**: Base classes have virtual destructors with `= default`

## Adding New Features

**New Expression Types**:
1. Forward declare in `src/ast/visitor.h`
2. Add `visit()` method to `ExprVisitor` interface
3. Define struct inheriting from `Expr` in `src/ast/expr.h`
4. Implement `accept()` method: `return v.visit(*this);`

**New Statement Types**:
- Follow same pattern with `StmtVisitor` and statement base class (not yet defined in codebase)

**New Value Types**:
- Add to `std::variant` in `src/common/value.h`
- Update `printValue()` and `isTruthy()` helper functions

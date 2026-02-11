#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/token.hpp"
#include "expr.hpp"
#include "visitor.hpp"

namespace izi {

// Base struct for all statements
struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

using StmtPtr = std::unique_ptr<Stmt>;

// Expression statement (e.g., "print(x);")
struct ExprStmt : public Stmt {
    ExprPtr expr;

    explicit ExprStmt(ExprPtr e)
        : expr(std::move(e)) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Block statement (e.g., "{ stmt1; stmt2; }")
struct BlockStmt : public Stmt {
    std::vector<StmtPtr> statements;

    explicit BlockStmt(std::vector<StmtPtr> stmts)
        : statements(std::move(stmts)) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Variable declaration (e.g., "var x = 10;")
struct VarStmt : public Stmt {
    std::string name;
    ExprPtr initializer;

    VarStmt(std::string n, ExprPtr init)
        : name(std::move(n)), initializer(std::move(init)) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// While loop (e.g., "while (condition) { body }")
struct WhileStmt : public Stmt {
    ExprPtr condition;
    StmtPtr body;

    WhileStmt(ExprPtr cond, StmtPtr b)
        : condition(std::move(cond)), body(std::move(b)) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Return statement (e.g., "return value;")
struct ReturnStmt : public Stmt {
    ExprPtr value;

    ReturnStmt(ExprPtr val)
        : value(std::move(val)) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Function declaration (e.g., "fn add(a, b) { return a + b; }")
struct FunctionStmt : public Stmt {
    std::string name;
    std::vector<std::string> params;
    std::vector<StmtPtr> body;

    FunctionStmt(std::string n, std::vector<std::string> p, std::vector<StmtPtr> b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// If statement (e.g., "if (condition) { thenBranch } else { elseBranch }")
struct IfStmt : public Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;

    IfStmt(ExprPtr cond, StmtPtr thenB, StmtPtr elseB)
        : condition(std::move(cond)),
          thenBranch(std::move(thenB)),
          elseBranch(std::move(elseB)) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Import statement with named/wildcard imports
// Supports:
// - import "module.iz";               (simple import)
// - import { name1, name2 } from "module.iz";  (named imports)
// - import * as name from "module.iz"; (wildcard import)
struct ImportStmt : public Stmt {
    std::string module;
    std::vector<std::string> namedImports; // Empty for simple imports
    std::string wildcardAlias;              // Empty unless "import * as name"
    bool isWildcard;
    
    // Simple import: import "module";
    explicit ImportStmt(std::string modName)
        : module(std::move(modName)), isWildcard(false) {}
    
    // Named imports: import { a, b } from "module";
    ImportStmt(std::string modName, std::vector<std::string> imports)
        : module(std::move(modName)), namedImports(std::move(imports)), isWildcard(false) {}
    
    // Wildcard import: import * as name from "module";
    ImportStmt(std::string modName, std::string alias, bool wildcard)
        : module(std::move(modName)), wildcardAlias(std::move(alias)), isWildcard(wildcard) {}

    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Export statement
// Supports:
// - export fn name() { ... }    (export function)
// - export var name = value;    (export variable)
struct ExportStmt : public Stmt {
    StmtPtr declaration; // The function or variable being exported
    
    explicit ExportStmt(StmtPtr decl)
        : declaration(std::move(decl)) {}
    
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Break statement (e.g., "break;")
struct BreakStmt : public Stmt {
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Continue statement (e.g., "continue;")
struct ContinueStmt : public Stmt {
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Try-Catch-Finally statement
struct TryStmt : public Stmt {
    StmtPtr tryBlock;
    std::string catchVariable;  // Variable name to bind the exception to (e.g., "e" in catch(e))
    StmtPtr catchBlock;         // Can be nullptr if no catch
    StmtPtr finallyBlock;       // Can be nullptr if no finally
    
    TryStmt(StmtPtr tryB, std::string catchVar, StmtPtr catchB, StmtPtr finallyB)
        : tryBlock(std::move(tryB)),
          catchVariable(std::move(catchVar)),
          catchBlock(std::move(catchB)),
          finallyBlock(std::move(finallyB)) {}
    
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Throw statement (e.g., "throw 'error message';")
struct ThrowStmt : public Stmt {
    ExprPtr value;
    
    explicit ThrowStmt(ExprPtr val)
        : value(std::move(val)) {}
    
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

}  // namespace izi

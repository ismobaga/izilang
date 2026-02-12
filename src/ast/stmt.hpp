#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/token.hpp"
#include "expr.hpp"
#include "visitor.hpp"
#include "type.hpp"
#include "pattern.hpp"

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

// Variable declaration (e.g., "var x = 10;" or "var x: Number = 10;" or "var [a, b] = [1, 2];")
struct VarStmt : public Stmt {
    std::string name;  // For simple variable declarations
    PatternPtr pattern;  // For destructuring declarations
    ExprPtr initializer;
    TypePtr typeAnnotation;  // Optional type annotation (v0.3)

    // Constructor for simple variable declaration
    VarStmt(std::string n, ExprPtr init, TypePtr type = nullptr)
        : name(std::move(n)), pattern(nullptr), initializer(std::move(init)), typeAnnotation(std::move(type)) {}
    
    // Constructor for destructuring declaration
    VarStmt(PatternPtr pat, ExprPtr init, TypePtr type = nullptr)
        : name(""), pattern(std::move(pat)), initializer(std::move(init)), typeAnnotation(std::move(type)) {}

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

// Function declaration (e.g., "fn add(a, b) { return a + b; }" or "fn add(a: Number, b: Number): Number { return a + b; }")
struct FunctionStmt : public Stmt {
    std::string name;
    std::vector<std::string> params;
    std::vector<StmtPtr> body;
    std::vector<TypePtr> paramTypes;   // Optional parameter type annotations (v0.3)
    TypePtr returnType;                 // Optional return type annotation (v0.3)

    FunctionStmt(std::string n, std::vector<std::string> p, std::vector<StmtPtr> b,
                 std::vector<TypePtr> pTypes = {}, TypePtr rType = nullptr)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)),
          paramTypes(std::move(pTypes)), returnType(std::move(rType)) {}

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
    Token keyword;  // The 'throw' token for error reporting
    ExprPtr value;
    
    ThrowStmt(Token kw, ExprPtr val)
        : keyword(std::move(kw)), value(std::move(val)) {}
    
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

// Class declaration (e.g., "class Point { ... }" or "class Dog extends Animal { ... }") (v0.3)
struct ClassStmt : public Stmt {
    std::string name;
    std::string superclass;  // Name of the superclass (empty string if no inheritance)
    std::vector<std::unique_ptr<VarStmt>> fields;      // Class fields (owned)
    std::vector<std::unique_ptr<FunctionStmt>> methods; // Class methods (owned)
    
    ClassStmt(std::string n, std::string super, std::vector<std::unique_ptr<VarStmt>> f, 
              std::vector<std::unique_ptr<FunctionStmt>> m)
        : name(std::move(n)), superclass(std::move(super)), fields(std::move(f)), methods(std::move(m)) {}
    
    void accept(StmtVisitor& v) override { v.visit(*this); }
};

}  // namespace izi

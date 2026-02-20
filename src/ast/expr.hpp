#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/token.hpp"
#include "common/value.hpp"
#include "visitor.hpp"
#include "pattern.hpp"

namespace izi {

// Base struct for all expressions
struct Expr {
    virtual ~Expr() = default;
    virtual Value accept(ExprVisitor& visitor) = 0;
};

using ExprPtr = std::unique_ptr<Expr>;
// Literal expressions
struct LiteralExpr : Expr {
    explicit LiteralExpr(Value v) : value(std::move(v)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }

    Value value;
};

// Variable reference
struct VariableExpr : Expr {
    std::string name;
    ExprPtr value;
    explicit VariableExpr(const std::string& n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Assign expressions (e.g., a = 56)
struct AssignExpr : Expr {
    std::string name;
    ExprPtr value;

    AssignExpr(std::string n, ExprPtr v) : name(std::move(n)), value(std::move(v)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Unary expressions (e.g., -a, !b)
struct UnaryExpr : Expr {
    Token op;
    ExprPtr right;

    UnaryExpr(Token o, ExprPtr r) : op(std::move(o)), right(std::move(r)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Binary expressions (e.g., a + b, a * b)
struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;

    BinaryExpr(ExprPtr l, Token o, ExprPtr r) : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Conditional (ternary) expressions (e.g., a ? b : c)
struct ConditionalExpr : Expr {
    ExprPtr condition;
    ExprPtr thenBranch;
    ExprPtr elseBranch;

    ConditionalExpr(ExprPtr cond, ExprPtr thenB, ExprPtr elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Grouping expressions (e.g., (a + b))
struct GroupingExpr : Expr {
    ExprPtr expression;

    explicit GroupingExpr(ExprPtr expr) : expression(std::move(expr)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Call expressions (e.g., functio())
struct CallExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> args;

    CallExpr(ExprPtr c, std::vector<ExprPtr> a) : callee(std::move(c)), args(std::move(a)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

struct ArrayExpr : Expr {
    std::vector<ExprPtr> elements;

    explicit ArrayExpr(std::vector<ExprPtr> elems) : elements(std::move(elems)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Spread expression for spreading arrays/maps (e.g., ...arr)
struct SpreadExpr : Expr {
    ExprPtr argument;

    explicit SpreadExpr(ExprPtr arg) : argument(std::move(arg)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

struct MapExpr : Expr {
    std::vector<std::pair<std::string, ExprPtr>> entries;

    explicit MapExpr(std::vector<std::pair<std::string, ExprPtr>> ents) : entries(std::move(ents)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

struct IndexExpr : Expr {
    ExprPtr collection;
    ExprPtr index;

    IndexExpr(ExprPtr coll, ExprPtr idx) : collection(std::move(coll)), index(std::move(idx)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

struct SetIndexExpr : Expr {
    ExprPtr collection;
    ExprPtr index;
    ExprPtr value;

    SetIndexExpr(ExprPtr coll, ExprPtr idx, ExprPtr val)
        : collection(std::move(coll)), index(std::move(idx)), value(std::move(val)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Property access expression (e.g., obj.field) (v0.3)
struct PropertyExpr : Expr {
    ExprPtr object;
    std::string property;

    PropertyExpr(ExprPtr obj, std::string prop) : object(std::move(obj)), property(std::move(prop)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Property set expression (e.g., obj.field = value) (v0.3)
struct SetPropertyExpr : Expr {
    ExprPtr object;
    std::string property;
    ExprPtr value;

    SetPropertyExpr(ExprPtr obj, std::string prop, ExprPtr val)
        : object(std::move(obj)), property(std::move(prop)), value(std::move(val)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// This expression for referencing the current instance (v0.3)
struct ThisExpr : Expr {
    ThisExpr() = default;

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Super expression for referencing the parent class (v0.3)
struct SuperExpr : Expr {
    std::string method;  // Name of the method to call on super

    explicit SuperExpr(std::string m) : method(std::move(m)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Forward declaration for statement types (from stmt.hpp)
// We only need FunctionExpr to reference these types, full definition is in stmt.hpp
struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

// Function expression for anonymous functions
// e.g., fn(a, b) { return a + b; }
struct FunctionExpr : Expr {
    std::vector<std::string> params;
    std::vector<StmtPtr> body;

    FunctionExpr(std::vector<std::string> p, std::vector<StmtPtr> b) : params(std::move(p)), body(std::move(b)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};

// Match case: pattern => expression, optionally with guard (if condition)
struct MatchCase {
    PatternPtr pattern;
    ExprPtr guard;  // Optional: if condition
    ExprPtr result;

    MatchCase(PatternPtr p, ExprPtr g, ExprPtr r) : pattern(std::move(p)), guard(std::move(g)), result(std::move(r)) {}

    // Move constructor
    MatchCase(MatchCase&&) = default;
    MatchCase& operator=(MatchCase&&) = default;

    // Delete copy constructor/assignment
    MatchCase(const MatchCase&) = delete;
    MatchCase& operator=(const MatchCase&) = delete;
};

// Match expression
// e.g., match value { 0 => "zero", x if x > 0 => "positive", _ => "unknown" }
struct MatchExpr : Expr {
    ExprPtr value;  // The value to match against
    std::vector<MatchCase> cases;

    MatchExpr(ExprPtr v, std::vector<MatchCase> c) : value(std::move(v)), cases(std::move(c)) {}

    Value accept(ExprVisitor& v) override { return v.visit(*this); }
};
}  // namespace izi
#pragma once

#include <memory>
#include <string>
#include "common/value.hpp"
#include "common/token.hpp"

namespace izi {

// Forward declaration
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

// Base class for all patterns
struct Pattern {
    virtual ~Pattern() = default;
};

using PatternPtr = std::unique_ptr<Pattern>;

// Wildcard pattern: _
struct WildcardPattern : Pattern {
    WildcardPattern() = default;
};

// Literal pattern: 0, 1, "hello", true, false, nil
struct LiteralPattern : Pattern {
    Value value;

    explicit LiteralPattern(Value v) : value(std::move(v)) {}
};

// Variable pattern: x
// Binds the matched value to a variable name
struct VariablePattern : Pattern {
    std::string name;

    explicit VariablePattern(std::string n) : name(std::move(n)) {}
};

// Array pattern for destructuring: [a, b, c]
struct ArrayPattern : Pattern {
    std::vector<PatternPtr> elements;

    explicit ArrayPattern(std::vector<PatternPtr> elems) : elements(std::move(elems)) {}
};

// Map pattern for destructuring: {name, age}
struct MapPattern : Pattern {
    std::vector<std::string> keys;

    explicit MapPattern(std::vector<std::string> k) : keys(std::move(k)) {}
};

}  // namespace izi

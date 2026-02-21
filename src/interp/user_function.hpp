#pragma once

#include <vector>
#include "common/value.hpp"
#include "common/callable.hpp"
#include "environment.hpp"
#include "ast/stmt.hpp"
#include "ast/expr.hpp"

namespace izi {
class Interpreter;

// Ownership note:
//   UserFunction holds a non-owning raw pointer to its closure Environment.
//   The Environment is owned by the EnvironmentArena inside the Interpreter
//   and therefore outlives the UserFunction for the duration of any given
//   interpreter session.  This avoids the shared_ptr reference cycle that
//   would otherwise exist between UserFunction and Environment.
class UserFunction : public Callable {
   public:
    // Constructor for function statements (named functions)
    UserFunction(FunctionStmt* declaration, Environment* closure)
        : decl(declaration), closure(closure), funcExpr(nullptr) {}

    // Constructor for function expressions (anonymous functions)
    UserFunction(FunctionExpr* expression, Environment* closure)
        : decl(nullptr), closure(closure), funcExpr(expression) {}

    std::string name() const override {
        if (decl) return decl->name.empty() ? "<anonymous>" : decl->name;
        return "<anonymous>";
    }

    int arity() const override {
        if (decl) return static_cast<int>(decl->params.size());
        if (funcExpr) return static_cast<int>(funcExpr->params.size());
        return 0;
    }

    Value call(Interpreter& interp, const std::vector<Value>& arguments) override;

    // Get the closure (needed for binding methods)
    Environment* getClosure() const { return closure; }

    // Get the function declaration (needed for creating bound methods)
    FunctionStmt* getDecl() const { return decl; }
    FunctionExpr* getFuncExpr() const { return funcExpr; }

   private:
    FunctionStmt* decl;  // For named functions (from statements)
    FunctionExpr* funcExpr;  // For anonymous functions (from expressions)
    Environment* closure;  // Non-owning; owned by EnvironmentArena
};

}  // namespace izi
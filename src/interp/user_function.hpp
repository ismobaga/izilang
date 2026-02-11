#pragma once

#include <memory>
#include <vector>
#include "common/value.hpp"
#include "common/callable.hpp"
#include "environment.hpp"
#include "ast/stmt.hpp"
#include "ast/expr.hpp"

namespace izi { 
    class Interpreter;

    class UserFunction : public Callable, public std::enable_shared_from_this<UserFunction> {
       public:
        // Constructor for function statements (named functions)
        UserFunction(FunctionStmt* declaration, std::shared_ptr<Environment> closure)
            : decl(declaration), closure(std::move(closure)), funcExpr(nullptr) {}

        // Constructor for function expressions (anonymous functions)
        UserFunction(FunctionExpr* expression, std::shared_ptr<Environment> closure)
            : decl(nullptr), closure(std::move(closure)), funcExpr(expression) {}

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

       private:
        FunctionStmt* decl;  // For named functions (from statements)
        FunctionExpr* funcExpr;  // For anonymous functions (from expressions)
        std::shared_ptr<Environment> closure;
    };

}  // namespace izi
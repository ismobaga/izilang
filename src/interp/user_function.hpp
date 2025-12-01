#pragma once

#include <memory>
#include <vector>
#include "common/value.hpp"
#include "common/callable.hpp"
#include "environment.hpp"
#include "ast/stmt.hpp"

namespace izi { 
    class Interpreter;

    class UserFunction : public Callable, public std::enable_shared_from_this<UserFunction> {
       public:
        UserFunction(FunctionStmt* declaration, Environment* closure)
            : decl(declaration), closure(closure) {}

        std::string name() const override { return decl->name; }
        int arity() const override { return static_cast<int>(decl->params.size()); }

        Value call(Interpreter& interp, const std::vector<Value>& arguments) override;

       private:
       FunctionStmt* decl;  // Ast node
        Environment* closure;
    };

}  // namespace izi
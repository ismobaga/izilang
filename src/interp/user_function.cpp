#include "user_function.hpp"
#include "interpreter.hpp"


namespace izi {
    Value UserFunction::call(Interpreter& interp, const std::vector<Value>& arguments) {
        auto localEnv = std::make_shared<Environment>(closure);

        // Get params and body from either decl or funcExpr
        const std::vector<std::string>* params = nullptr;
        const std::vector<StmtPtr>* body = nullptr;
        
        if (decl) {
            params = &decl->params;
            body = &decl->body;
        } else if (funcExpr) {
            params = &funcExpr->params;
            body = &funcExpr->body;
        } else {
            throw std::runtime_error("Invalid UserFunction: no declaration or expression");
        }

        for (size_t i = 0; i < params->size(); ++i) {
            const std::string& paramName = (*params)[i];
            Value argVal = (i < arguments.size()) ? arguments[i] : Nil{};

            localEnv->define(paramName, argVal);

        }

        try {
            interp.executeBlock(*body, localEnv);
        } catch (const ReturnSignal& returnValue) {
            return returnValue.value;
        }

        return Nil{};
    }
}  // namespace izi
#include "user_function.hpp"
#include "interpreter.hpp"

namespace izi {
Value UserFunction::call(Interpreter& interp, const std::vector<Value>& arguments) {
    // Check call depth to prevent stack overflow
    if (interp.callDepth >= MAX_CALL_DEPTH) {
        throw std::runtime_error("Stack overflow: Maximum call depth of " + std::to_string(MAX_CALL_DEPTH) +
                                 " exceeded.");
    }

    // Increment call depth
    interp.callDepth++;

    auto localEnv = interp.arena_.create(closure);

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
        interp.callDepth--;  // Restore call depth before throwing
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
        interp.callDepth--;  // Restore call depth on return
        return returnValue.value;
    } catch (...) {
        interp.callDepth--;  // Restore call depth on exception
        throw;
    }

    interp.callDepth--;  // Restore call depth on normal exit
    return Nil{};
}
}  // namespace izi
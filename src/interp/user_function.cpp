#include "user_function.hpp"
#include "interpreter.hpp"


namespace izi {
    Value UserFunction::call(Interpreter& interp, const std::vector<Value>& arguments) {
        Environment localEnv(closure);

        for (size_t i = 0; i < decl->params.size(); ++i) {
            const std::string& paramName = decl->params[i];
            Value argVal = (i < arguments.size()) ? arguments[i] : Nil{};

            localEnv.define(paramName, argVal);

        }

        try {
            interp.executeBlock(decl->body, &localEnv);
        } catch (const ReturnSignal& returnValue) {
            return returnValue.value;
        }

        return Nil{};
    }
}  // namespace izi
#include "izi_class.hpp"
#include "interpreter.hpp"
#include "user_function.hpp"

namespace izi {

Value BoundMethod::call(Interpreter& interp, const std::vector<Value>& arguments) {
    // To properly bind 'this', we create a new environment with 'this' defined
    // and the method's closure as its parent
    
    auto userFunc = std::dynamic_pointer_cast<UserFunction>(method);
    if (!userFunc) {
        // If it's not a UserFunction, just call it directly
        return method->call(interp, arguments);
    }
    
    // Create a new environment with 'this' defined, using the method's closure as parent
    auto thisEnv = std::make_shared<Environment>(userFunc->getClosure());
    thisEnv->define("this", instance);
    
    // Create a temporary UserFunction with the new closure
    std::shared_ptr<UserFunction> boundFunc;
    if (userFunc->getDecl()) {
        boundFunc = std::make_shared<UserFunction>(userFunc->getDecl(), thisEnv);
    } else if (userFunc->getFuncExpr()) {
        boundFunc = std::make_shared<UserFunction>(userFunc->getFuncExpr(), thisEnv);
    } else {
        throw std::runtime_error("Invalid UserFunction: no declaration or expression");
    }
    
    // Call the bound function
    return boundFunc->call(interp, arguments);
}

int IziClass::arity() const {
    // Check if there's a constructor method
    auto it = methods.find("constructor");
    if (it != methods.end()) {
        // Get the arity from the constructor
        if (std::holds_alternative<std::shared_ptr<Callable>>(it->second)) {
            auto constructor = std::get<std::shared_ptr<Callable>>(it->second);
            return constructor->arity();
        }
    }
    return 0;  // No constructor means no arguments
}

Value IziClass::call(Interpreter& interp, const std::vector<Value>& arguments) {
    // Create a new instance
    auto instance = std::make_shared<Instance>(shared_from_this());
    
    // Initialize fields with their default values
    for (const auto& fieldName : fieldNames) {
        auto it = fieldDefaults.find(fieldName);
        if (it != fieldDefaults.end()) {
            instance->fields[fieldName] = it->second;
        } else {
            instance->fields[fieldName] = Nil{};
        }
    }
    
    // If there's a constructor, call it with 'this' bound
    auto constructorIt = methods.find("constructor");
    if (constructorIt != methods.end()) {
        Value constructor = getMethod("constructor", instance);
        if (std::holds_alternative<std::shared_ptr<Callable>>(constructor)) {
            auto constructorCallable = std::get<std::shared_ptr<Callable>>(constructor);
            constructorCallable->call(interp, arguments);
        }
    }
    
    return instance;
}

Value IziClass::getMethod(const std::string& name, std::shared_ptr<Instance> instance) {
    auto it = methods.find(name);
    if (it == methods.end()) {
        return Nil{};
    }
    
    // Bind the method to the instance
    if (std::holds_alternative<std::shared_ptr<Callable>>(it->second)) {
        auto callable = std::get<std::shared_ptr<Callable>>(it->second);
        return std::make_shared<BoundMethod>(instance, callable);
    }
    
    return it->second;
}

}  // namespace izi

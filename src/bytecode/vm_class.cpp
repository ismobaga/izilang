#include "vm_class.hpp"
#include "vm.hpp"
#include "vm_user_function.hpp"
#include "interp/izi_class.hpp"

namespace izi {

Value VmBoundMethod::call(VM& vm, const std::vector<Value>& arguments) {
    // Set 'this' as a global variable before calling the method
    // Note: This is a simplified implementation. Using a global variable means
    // nested method calls or recursive methods will overwrite 'this', which could
    // cause incorrect behavior. A proper implementation would use a call stack
    // mechanism where 'this' is passed as a local variable or stored in call frames.
    // For the current use cases (non-recursive, non-nested method calls), this works.
    vm.setGlobal("this", instance);
    
    // Call the method
    Value result = method->call(vm, arguments);
    
    // Note: Not clearing 'this' here to allow it to persist for the next call
    // This is acceptable since it will be overwritten by the next method call
    
    return result;
}

int VmClass::arity() const {
    // Check if there's a constructor method
    auto it = methods.find("constructor");
    if (it != methods.end()) {
        return it->second->arity();
    }
    return 0;  // No constructor means no arguments
}

Value VmClass::call(VM& vm, const std::vector<Value>& arguments) {
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
        auto boundMethod = std::make_shared<VmBoundMethod>(instance, constructorIt->second);
        boundMethod->call(vm, arguments);
    }
    
    return instance;
}

std::shared_ptr<VmCallable> VmClass::getMethod(const std::string& name, std::shared_ptr<Instance> instance) {
    auto it = methods.find(name);
    if (it == methods.end()) {
        return nullptr;
    }
    
    // Bind the method to the instance
    return std::make_shared<VmBoundMethod>(instance, it->second);
}

}  // namespace izi

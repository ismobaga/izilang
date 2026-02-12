#include "vm_class.hpp"
#include "vm.hpp"
#include "vm_user_function.hpp"
#include "interp/izi_class.hpp"

namespace izi {

Value VmBoundMethod::call(VM& vm, const std::vector<Value>& arguments) {
    // Set 'this' as a global variable before calling the method
    // This is a simplified approach - ideally we'd use local scopes
    vm.setGlobal("this", instance);
    
    // Call the method
    Value result = method->call(vm, arguments);
    
    // Clean up 'this' after the call (optional - could be left for next call)
    // vm.setGlobal("this", Nil{});
    
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

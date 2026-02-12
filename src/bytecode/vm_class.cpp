#include "vm_class.hpp"
#include "vm.hpp"
#include "vm_user_function.hpp"
#include "interp/izi_class.hpp"

namespace izi {

Value VmBoundMethod::call(VM& vm, const std::vector<Value>& arguments) {
    // For VM execution, we need to set up 'this' in the local scope
    // Since the method is a VmUserFunction, we'll handle 'this' specially in the VM
    
    // For now, we need to push 'this' onto the stack before calling the method
    // The method compilation should expect 'this' as a hidden first parameter
    
    // We'll store the instance in a temporary location and call the method
    // This is a simplified approach - ideally we'd handle this in the call frame
    std::vector<Value> argsWithThis;
    argsWithThis.push_back(instance);
    argsWithThis.insert(argsWithThis.end(), arguments.begin(), arguments.end());
    
    return method->call(vm, arguments);
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

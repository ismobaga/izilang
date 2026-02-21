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

    // Set 'super' to the owning class's superclass so that super.method() works
    if (ownerClass && ownerClass->superclass) {
        vm.setGlobal("super", ownerClass->superclass);
    }

    // Call the method
    Value result = method->call(vm, arguments);

    return result;
}

int VmClass::arity() const {
    // Check if there's a constructor method
    auto it = methods.find("constructor");
    if (it != methods.end()) {
        return it->second->arity();
    }
    // Check superclass constructor
    if (superclass) {
        return superclass->arity();
    }
    return 0;  // No constructor means no arguments
}

// Helper to recursively initialize fields from the entire inheritance chain
static void initFieldsRecursive(const VmClass* klass, std::shared_ptr<Instance> instance) {
    if (klass->superclass) {
        initFieldsRecursive(klass->superclass.get(), instance);
    }
    for (const auto& fieldName : klass->fieldNames) {
        auto it = klass->fieldDefaults.find(fieldName);
        if (it != klass->fieldDefaults.end()) {
            instance->fields[fieldName] = it->second;
        } else if (instance->fields.find(fieldName) == instance->fields.end()) {
            instance->fields[fieldName] = Nil{};
        }
    }
}

Value VmClass::call(VM& vm, const std::vector<Value>& arguments) {
    // Create a new instance
    auto instance = std::make_shared<Instance>(shared_from_this());

    // Initialize fields from the entire inheritance chain
    initFieldsRecursive(this, instance);

    // If there's a constructor, call it with 'this' bound
    auto constructorIt = methods.find("constructor");
    if (constructorIt != methods.end()) {
        auto boundMethod = std::make_shared<VmBoundMethod>(instance, constructorIt->second, shared_from_this());
        boundMethod->call(vm, arguments);
    } else if (superclass) {
        // Inherit constructor from superclass chain: find and call it with 'this' bound
        std::shared_ptr<VmClass> ownerKlass = superclass;
        while (ownerKlass) {
            auto superConstructorIt = ownerKlass->methods.find("constructor");
            if (superConstructorIt != ownerKlass->methods.end()) {
                auto boundMethod = std::make_shared<VmBoundMethod>(instance, superConstructorIt->second, ownerKlass);
                boundMethod->call(vm, arguments);
                break;
            }
            ownerKlass = ownerKlass->superclass;
        }
    }

    return instance;
}

std::shared_ptr<VmCallable> VmClass::getMethod(const std::string& name, std::shared_ptr<Instance> instance) {
    auto it = methods.find(name);
    if (it != methods.end()) {
        // Bind the method to the instance, passing this class as owner for 'super' resolution
        return std::make_shared<VmBoundMethod>(instance, it->second, shared_from_this());
    }

    // Search superclass chain
    if (superclass) {
        return superclass->getMethod(name, instance);
    }

    return nullptr;
}

}  // namespace izi

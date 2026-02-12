#pragma once

#include "mv_callable.hpp"
#include "chunk.hpp"
#include "common/value.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace izi {

// Forward declarations
class VmClass;

// Represents an instance of a class in the VM
// Note: Instance is already defined in common/value.hpp via izi_class.hpp
// We'll use the same Instance struct for both interpreter and VM

// Binds a method to an instance in the VM
class VmBoundMethod : public VmCallable {
public:
    std::shared_ptr<Instance> instance;
    std::shared_ptr<VmCallable> method;
    
    VmBoundMethod(std::shared_ptr<Instance> inst, std::shared_ptr<VmCallable> meth)
        : instance(std::move(inst)), method(std::move(meth)) {}
    
    std::string name() const override { return method->name(); }
    
    int arity() const override { return method->arity(); }
    
    Value call(VM& vm, const std::vector<Value>& arguments) override;
};

// Represents a class definition in the VM (callable to construct instances)
class VmClass : public VmCallable, public std::enable_shared_from_this<VmClass> {
public:
    std::string className;
    std::unordered_map<std::string, std::shared_ptr<VmCallable>> methods;
    std::vector<std::string> fieldNames;
    std::unordered_map<std::string, Value> fieldDefaults;
    
    VmClass(std::string name, 
            std::vector<std::string> fields,
            std::unordered_map<std::string, Value> defaults,
            std::unordered_map<std::string, std::shared_ptr<VmCallable>> meths)
        : className(std::move(name)),
          methods(std::move(meths)),
          fieldNames(std::move(fields)),
          fieldDefaults(std::move(defaults)) {}
    
    std::string name() const override { return className; }
    
    int arity() const override;
    
    Value call(VM& vm, const std::vector<Value>& arguments) override;
    
    std::shared_ptr<VmCallable> getMethod(const std::string& name, std::shared_ptr<Instance> instance);
};

}  // namespace izi

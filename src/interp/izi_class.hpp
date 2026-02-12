#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>

#include "common/callable.hpp"
#include "common/value.hpp"
#include "ast/stmt.hpp"
#include "environment.hpp"

namespace izi {

class Interpreter;
class IziClass;
class VmClass;

// Represents an instance of a class
struct Instance {
    std::variant<std::shared_ptr<IziClass>, std::shared_ptr<VmClass>> klass;
    std::unordered_map<std::string, Value> fields;
    
    explicit Instance(std::shared_ptr<IziClass> k) : klass(std::move(k)) {}
    explicit Instance(std::shared_ptr<VmClass> k) : klass(std::move(k)) {}
};

// Binds a method to an instance
class BoundMethod : public Callable {
public:
    std::shared_ptr<Instance> instance;
    std::shared_ptr<Callable> method;
    
    BoundMethod(std::shared_ptr<Instance> inst, std::shared_ptr<Callable> meth)
        : instance(std::move(inst)), method(std::move(meth)) {}
    
    std::string name() const override { return method->name(); }
    
    int arity() const override { return method->arity(); }
    
    Value call(Interpreter& interp, const std::vector<Value>& arguments) override;
};

// Represents a class definition (callable to construct instances)
class IziClass : public Callable, public std::enable_shared_from_this<IziClass> {
public:
    std::string className;
    std::unordered_map<std::string, Value> methods;
    std::vector<std::string> fieldNames;
    std::unordered_map<std::string, Value> fieldDefaults;
    
    IziClass(std::string name, 
             std::vector<std::string> fields,
             std::unordered_map<std::string, Value> defaults,
             std::unordered_map<std::string, Value> meths)
        : className(std::move(name)),
          methods(std::move(meths)),
          fieldNames(std::move(fields)),
          fieldDefaults(std::move(defaults)) {}
    
    std::string name() const override { return className; }
    
    int arity() const override;
    
    Value call(Interpreter& interp, const std::vector<Value>& arguments) override;
    
    Value getMethod(const std::string& name, std::shared_ptr<Instance> instance);
};

}  // namespace izi

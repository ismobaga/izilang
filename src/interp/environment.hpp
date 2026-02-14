#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "common/value.hpp"

namespace izi {

class Environment : public std::enable_shared_from_this<Environment> {
   public:
    Environment() = default;

    explicit Environment(std::shared_ptr<Environment> enclosing)
        : parent(std::move(enclosing)) {}

    void define(const std::string& name, const Value& value) {
        values[name] = value;
    }

    Value get(const std::string& name) const {
        auto it = values.find(name);
        if (it != values.end()) {
            return it->second;
        }

        if (parent != nullptr) {
            return parent->get(name);
        }

        throw std::runtime_error("Undefined variable '" + name + "'.");
    }

    void assign(const std::string& name, const Value& value) {
        auto it = values.find(name);
        if (it != values.end()) {
            it->second = value;
            return;
        }

        if (parent != nullptr) {
            parent->assign(name, value);
            return;
        }

        throw std::runtime_error("Undefined variable '" + name + "'.");
    }
    
    // Get all variables in this environment (for REPL :vars command)
    const std::unordered_map<std::string, Value>& getAll() const {
        return values;
    }
    
    std::shared_ptr<Environment> getParent() const {
        return parent;
    }

   private:
    std::unordered_map<std::string, Value> values;
    std::shared_ptr<Environment> parent;
};

}  // namespace izi
#pragma once

#include <string>
#include <unordered_map>
#include <stdexcept>
#include "common/value.hpp"

namespace izi {

// Ownership note:
//   Environment instances are owned exclusively by an EnvironmentArena.
//   The `parent` pointer is therefore a non-owning raw pointer – the arena
//   guarantees that parent environments outlive their children for the
//   duration of interpreter execution.  Do not delete Environment objects
//   directly; let the arena manage their lifetime.
class Environment {
   public:
    Environment() = default;

    explicit Environment(Environment* enclosing) : parent(enclosing) {}

    void define(const std::string& name, const Value& value) { values[name] = value; }

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
    const std::unordered_map<std::string, Value>& getAll() const { return values; }

    Environment* getParent() const { return parent; }

   private:
    std::unordered_map<std::string, Value> values;
    Environment* parent = nullptr;
};

}  // namespace izi
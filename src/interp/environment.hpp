#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "common/value.hpp"

namespace izi {

class Environment {
   public:
    Environment() : parent(nullptr) {}

    explicit Environment(Environment* enclosing = nullptr)
        : parent(enclosing) {}

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

   private:
    std::unordered_map<std::string, Value> values;
    Environment* parent;
};

}  // namespace izi
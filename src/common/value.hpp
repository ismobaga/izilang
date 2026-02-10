#pragma once

#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace izi {
using Nil = std::monostate;

class Callable;
class VmCallable;
struct Array;
struct Map;

using Value = std::variant<
    Nil,
    bool,
    double,
    std::string,
    std::shared_ptr<Array>,
    std::shared_ptr<Map>,
    std::shared_ptr<Callable>,
    std::shared_ptr<VmCallable>

    >;


// Forward declare to avoid circular dependency
}  // namespace izi

// Include after Value definition to avoid circular dependency
#include "callable.hpp"



namespace izi {

        struct Array {
        std::vector<Value> elements;
    };
    struct Map {
        std::unordered_map<std::string, Value> entries;
    };


void printValue(const Value &v); // forward

inline void printArray(const Array& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr.elements.size(); ++i) {
        printValue(arr.elements[i]);
        if (i < arr.elements.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

inline void printMap(const Map& map) {
    std::cout << "{";
    size_t count = 0;
    for (const auto& [key, value] : map.entries) {
        std::cout << key << ": ";
        printValue(value);
        if (count < map.entries.size() - 1) {
            std::cout << ", ";
        }
        ++count;
    }
    std::cout << "}";
}   



void printValue(const Value& v);

inline bool isTruthy(const Value& v) {
    if (std::holds_alternative<Nil>(v)) {
        return false;
    } else if (std::holds_alternative<bool>(v)) {
        return std::get<bool>(v);
    } else if (std::holds_alternative<double>(v)) {
        return std::get<double>(v) != 0.0;
    } else if (std::holds_alternative<std::string>(v)) {
        return !std::get<std::string>(v).empty();
    } else if (std::holds_alternative<std::shared_ptr<Array>>(v)) {
        return !std::get<std::shared_ptr<Array>>(v)->elements.empty();
    } else if (std::holds_alternative<std::shared_ptr<Map>>(v)) {
        return !std::get<std::shared_ptr<Map>>(v)->entries.empty();
    }
    return false;
}

inline double asNumber(const Value& v) {
    if (!std::holds_alternative<double>(v)) {
        throw std::runtime_error("Expected number");
    }

    return std::get<double>(v);
}

inline std::string getTypeName(const Value& v) {
    if (std::holds_alternative<Nil>(v)) {
        return "nil";
    } else if (std::holds_alternative<bool>(v)) {
        return "boolean";
    } else if (std::holds_alternative<double>(v)) {
        return "number";
    } else if (std::holds_alternative<std::string>(v)) {
        return "string";
    } else if (std::holds_alternative<std::shared_ptr<Array>>(v)) {
        return "array";
    } else if (std::holds_alternative<std::shared_ptr<Map>>(v)) {
        return "map";
    } else if (std::holds_alternative<std::shared_ptr<Callable>>(v)) {
        return "function";
    } else if (std::holds_alternative<std::shared_ptr<VmCallable>>(v)) {
        return "function";
    }
    return "unknown";
}

}  // namespace izi
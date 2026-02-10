#include "vm_native.hpp"
#include <chrono>
#include <iostream>

namespace izi {

Value vmNativePrint(VM& vm, const std::vector<Value>& arguments) {
    for (size_t i = 0; i < arguments.size(); ++i) {
        printValue(arguments[i]);
        if (i + 1 < arguments.size()) std::cout << " ";
    }
    std::cout << "\n";
    return Nil{};
}

Value vmNativeLen(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("len() takes exactly one argument.");
    }
    const Value& arg = arguments[0];
    if (std::holds_alternative<std::shared_ptr<Array>>(arg)) {
        auto arr = std::get<std::shared_ptr<Array>>(arg);
        return static_cast<double>(arr->elements.size());
    } else if (std::holds_alternative<std::shared_ptr<Map>>(arg)) {
        auto map = std::get<std::shared_ptr<Map>>(arg);
        return static_cast<double>(map->entries.size());
    } else if (std::holds_alternative<std::string>(arg)) {
        auto str = std::get<std::string>(arg);
        return static_cast<double>(str.size());
    }
    return Nil{};
}

Value vmNativeClock(VM& vm, const std::vector<Value>& arguments) {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return static_cast<double>(ms) / 1000.0;
}

Value vmNativePush(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("push() takes exactly two arguments.");
    }
    const Value& arrVal = arguments[0];
    const Value& elem = arguments[1];
    if (!std::holds_alternative<std::shared_ptr<Array>>(arrVal)) {
        throw std::runtime_error("First argument to push() must be an array.");
    }
    auto arr = std::get<std::shared_ptr<Array>>(arrVal);
    arr->elements.push_back(elem);
    return arr;
}

Value vmNativePop(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("pop() takes exactly one argument.");
    }
    const Value& arrVal = arguments[0];
    if (!std::holds_alternative<std::shared_ptr<Array>>(arrVal)) {
        throw std::runtime_error("Argument to pop() must be an array.");
    }
    auto arr = std::get<std::shared_ptr<Array>>(arrVal);
    if (arr->elements.empty()) {
        throw std::runtime_error("Cannot pop from an empty array.");
    }
    Value elem = arr->elements.back();
    arr->elements.pop_back();
    return elem;
}

Value vmNativeKeys(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("keys() takes exactly one argument.");
    }
    const Value& mapVal = arguments[0];
    if (!std::holds_alternative<std::shared_ptr<Map>>(mapVal)) {
        throw std::runtime_error("Argument to keys() must be a map.");
    }
    auto map = std::get<std::shared_ptr<Map>>(mapVal);
    auto keysArray = std::make_shared<Array>();
    for (const auto& [key, _] : map->entries) {
        keysArray->elements.push_back(key);
    }
    return keysArray;
}

Value vmNativeValues(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("values() takes exactly one argument.");
    }
    const Value& mapVal = arguments[0];
    if (!std::holds_alternative<std::shared_ptr<Map>>(mapVal)) {
        throw std::runtime_error("Argument to values() must be a map.");
    }
    auto map = std::get<std::shared_ptr<Map>>(mapVal);
    auto valuesArray = std::make_shared<Array>();
    for (const auto& [_, value] : map->entries) {
        valuesArray->elements.push_back(value);
    }
    return valuesArray;
}

Value vmNativeHasKey(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("hasKey() takes exactly two arguments.");
    }
    const Value& mapVal = arguments[0];
    const Value& keyVal = arguments[1];
    if (!std::holds_alternative<std::shared_ptr<Map>>(mapVal)) {
        throw std::runtime_error("First argument to hasKey() must be a map.");
    }
    if (!std::holds_alternative<std::string>(keyVal)) {
        throw std::runtime_error("Second argument to hasKey() must be a string.");
    }
    auto map = std::get<std::shared_ptr<Map>>(mapVal);
    std::string key = std::get<std::string>(keyVal);
    bool hasKey = (map->entries.find(key) != map->entries.end());
    return hasKey;
}

void registerVmNatives(VM& vm) {
    vm.setGlobal("print", std::make_shared<VmNativeFunction>("print", -1, vmNativePrint));
    vm.setGlobal("len", std::make_shared<VmNativeFunction>("len", 1, vmNativeLen));
    vm.setGlobal("clock", std::make_shared<VmNativeFunction>("clock", 0, vmNativeClock));
    vm.setGlobal("push", std::make_shared<VmNativeFunction>("push", 2, vmNativePush));
    vm.setGlobal("pop", std::make_shared<VmNativeFunction>("pop", 1, vmNativePop));
    vm.setGlobal("keys", std::make_shared<VmNativeFunction>("keys", 1, vmNativeKeys));
    vm.setGlobal("values", std::make_shared<VmNativeFunction>("values", 1, vmNativeValues));
    vm.setGlobal("hasKey", std::make_shared<VmNativeFunction>("hasKey", 2, vmNativeHasKey));
}

} // namespace izi

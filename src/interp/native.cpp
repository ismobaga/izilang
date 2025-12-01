#include "native.hpp"
#include "interpreter.hpp"
#include <chrono>

namespace izi {
auto nativePrint(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    for (size_t i =0; i < arguments.size(); ++i) {
        printValue(arguments[i]);
        if(i+1 < arguments.size()) std::cout << " ";
    }
    std::cout << "\n";
    return Nil{};
}

auto nativeClock(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return static_cast<double>(ms) / 1000.0;
}

auto nativeLen(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
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

auto nativePush(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
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

auto nativePop(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
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
auto nativeKeys(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
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

auto nativeValues(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
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

auto nativeHasKey(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
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


void registerNativeFunctions(Interpreter& interp) {
    interp.defineGlobal("print", Value{std::make_shared<NativeFunction>(
        "print", -1, nativePrint)});
    interp.defineGlobal("clock", Value{std::make_shared<NativeFunction>(
        "clock", 0, nativeClock)});

    interp.defineGlobal("len", Value{std::make_shared<NativeFunction>(
        "len", 1, nativeLen)});
    interp.defineGlobal("push", Value{std::make_shared<NativeFunction>(
        "push", 2, nativePush)});
    interp.defineGlobal("pop", Value{std::make_shared<NativeFunction>(
        "pop", 1, nativePop)});
    interp.defineGlobal("keys", Value{std::make_shared<NativeFunction>(
        "keys", 1, nativeKeys)});
    interp.defineGlobal("values", Value{std::make_shared<NativeFunction>(
        "values", 1, nativeValues)});
    interp.defineGlobal("hasKey", Value{std::make_shared<NativeFunction>(
        "hasKey", 2, nativeHasKey)});   
        
    }

    

}
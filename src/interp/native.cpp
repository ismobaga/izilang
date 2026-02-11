#include "native.hpp"
#include "interpreter.hpp"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <sys/stat.h>

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
    } else if (std::holds_alternative<std::shared_ptr<Set>>(arg)) {
        auto set = std::get<std::shared_ptr<Set>>(arg);
        return static_cast<double>(set->values.size());
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

auto nativeShift(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("shift() takes exactly one argument.");
    }
    const Value& arrVal = arguments[0];
    if (!std::holds_alternative<std::shared_ptr<Array>>(arrVal)) {
        throw std::runtime_error("Argument to shift() must be an array.");
    }
    auto arr = std::get<std::shared_ptr<Array>>(arrVal);
    if (arr->elements.empty()) {
        throw std::runtime_error("Cannot shift from an empty array.");
    }
    Value elem = arr->elements.front();
    arr->elements.erase(arr->elements.begin());
    return elem;
}

auto nativeUnshift(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("unshift() takes exactly two arguments.");
    }
    const Value& arrVal = arguments[0];
    const Value& elem = arguments[1];
    if (!std::holds_alternative<std::shared_ptr<Array>>(arrVal)) {
        throw std::runtime_error("First argument to unshift() must be an array.");
    }
    auto arr = std::get<std::shared_ptr<Array>>(arrVal);
    arr->elements.insert(arr->elements.begin(), elem);
    return arr;
}

auto nativeSplice(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 2 || arguments.size() > 3) {
        throw std::runtime_error("splice() takes 2 or 3 arguments.");
    }
    const Value& arrVal = arguments[0];
    if (!std::holds_alternative<std::shared_ptr<Array>>(arrVal)) {
        throw std::runtime_error("First argument to splice() must be an array.");
    }
    
    auto arr = std::get<std::shared_ptr<Array>>(arrVal);
    size_t start = static_cast<size_t>(asNumber(arguments[1]));
    
    if (start >= arr->elements.size()) {
        return std::make_shared<Array>();
    }
    
    size_t deleteCount;
    if (arguments.size() == 3) {
        deleteCount = static_cast<size_t>(asNumber(arguments[2]));
    } else {
        deleteCount = arr->elements.size() - start;
    }
    
    // Create result array with removed elements
    auto result = std::make_shared<Array>();
    size_t end = std::min(start + deleteCount, arr->elements.size());
    
    for (size_t i = start; i < end; ++i) {
        result->elements.push_back(arr->elements[i]);
    }
    
    // Remove elements from original array
    arr->elements.erase(arr->elements.begin() + start, arr->elements.begin() + end);
    
    return result;
}

auto nativeHas(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    // Alias for hasKey() for map compatibility
    return nativeHasKey(interp, arguments);
}

auto nativeDelete(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("delete() takes exactly two arguments.");
    }
    const Value& mapVal = arguments[0];
    const Value& keyVal = arguments[1];
    if (!std::holds_alternative<std::shared_ptr<Map>>(mapVal)) {
        throw std::runtime_error("First argument to delete() must be a map.");
    }
    if (!std::holds_alternative<std::string>(keyVal)) {
        throw std::runtime_error("Second argument to delete() must be a string.");
    }
    auto map = std::get<std::shared_ptr<Map>>(mapVal);
    std::string key = std::get<std::string>(keyVal);
    bool existed = (map->entries.find(key) != map->entries.end());
    if (existed) {
        map->entries.erase(key);
    }
    return existed;
}

auto nativeEntries(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("entries() takes exactly one argument.");
    }
    const Value& mapVal = arguments[0];
    if (!std::holds_alternative<std::shared_ptr<Map>>(mapVal)) {
        throw std::runtime_error("Argument to entries() must be a map.");
    }
    auto map = std::get<std::shared_ptr<Map>>(mapVal);
    auto entriesArray = std::make_shared<Array>();
    for (const auto& [key, value] : map->entries) {
        auto entry = std::make_shared<Array>();
        entry->elements.push_back(key);
        entry->elements.push_back(value);
        entriesArray->elements.push_back(entry);
    }
    return entriesArray;
}

// ============ Set functions ============

auto nativeSetAdd(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("setAdd() takes exactly two arguments.");
    }
    const Value& setVal = arguments[0];
    const Value& valueVal = arguments[1];
    if (!std::holds_alternative<std::shared_ptr<Set>>(setVal)) {
        throw std::runtime_error("First argument to setAdd() must be a set.");
    }
    
    auto set = std::get<std::shared_ptr<Set>>(setVal);
    
    // Convert value to string for key (using a simple serialization)
    std::string key;
    if (std::holds_alternative<std::string>(valueVal)) {
        key = std::get<std::string>(valueVal);
    } else if (std::holds_alternative<double>(valueVal)) {
        // Use stringstream for consistent number formatting
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(15) << std::get<double>(valueVal);
        key = oss.str();
    } else if (std::holds_alternative<bool>(valueVal)) {
        key = std::get<bool>(valueVal) ? "true" : "false";
    } else if (std::holds_alternative<Nil>(valueVal)) {
        key = "nil";
    } else {
        throw std::runtime_error("setAdd() only supports primitive types (string, number, boolean, nil), but got: " + getTypeName(valueVal));
    }
    
    set->values[key] = valueVal;
    return set;
}

auto nativeSetHas(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("setHas() takes exactly two arguments.");
    }
    const Value& setVal = arguments[0];
    const Value& valueVal = arguments[1];
    if (!std::holds_alternative<std::shared_ptr<Set>>(setVal)) {
        throw std::runtime_error("First argument to setHas() must be a set.");
    }
    
    auto set = std::get<std::shared_ptr<Set>>(setVal);
    
    // Convert value to string for key lookup
    std::string key;
    if (std::holds_alternative<std::string>(valueVal)) {
        key = std::get<std::string>(valueVal);
    } else if (std::holds_alternative<double>(valueVal)) {
        // Use stringstream for consistent number formatting
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(15) << std::get<double>(valueVal);
        key = oss.str();
    } else if (std::holds_alternative<bool>(valueVal)) {
        key = std::get<bool>(valueVal) ? "true" : "false";
    } else if (std::holds_alternative<Nil>(valueVal)) {
        key = "nil";
    } else {
        return false;
    }
    
    return (set->values.find(key) != set->values.end());
}

auto nativeSetDelete(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("setDelete() takes exactly two arguments.");
    }
    const Value& setVal = arguments[0];
    const Value& valueVal = arguments[1];
    if (!std::holds_alternative<std::shared_ptr<Set>>(setVal)) {
        throw std::runtime_error("First argument to setDelete() must be a set.");
    }
    
    auto set = std::get<std::shared_ptr<Set>>(setVal);
    
    // Convert value to string for key lookup
    std::string key;
    if (std::holds_alternative<std::string>(valueVal)) {
        key = std::get<std::string>(valueVal);
    } else if (std::holds_alternative<double>(valueVal)) {
        // Use stringstream for consistent number formatting
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(15) << std::get<double>(valueVal);
        key = oss.str();
    } else if (std::holds_alternative<bool>(valueVal)) {
        key = std::get<bool>(valueVal) ? "true" : "false";
    } else if (std::holds_alternative<Nil>(valueVal)) {
        key = "nil";
    } else {
        return false;
    }
    
    bool existed = (set->values.find(key) != set->values.end());
    if (existed) {
        set->values.erase(key);
    }
    return existed;
}

auto nativeSetSize(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("setSize() takes exactly one argument.");
    }
    const Value& setVal = arguments[0];
    if (!std::holds_alternative<std::shared_ptr<Set>>(setVal)) {
        throw std::runtime_error("Argument to setSize() must be a set.");
    }
    auto set = std::get<std::shared_ptr<Set>>(setVal);
    return static_cast<double>(set->values.size());
}

auto nativeSet(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 0) {
        throw std::runtime_error("Set() takes no arguments.");
    }
    return std::make_shared<Set>();
}

// ============ std.math functions ============

auto nativeSqrt(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("sqrt() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    if (val < 0) {
        throw std::runtime_error("sqrt() argument must be non-negative.");
    }
    return std::sqrt(val);
}

auto nativePow(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("pow() takes exactly two arguments.");
    }
    double base = asNumber(arguments[0]);
    double exp = asNumber(arguments[1]);
    return std::pow(base, exp);
}

auto nativeAbs(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("abs() takes exactly one argument.");
    }
    return std::abs(asNumber(arguments[0]));
}

auto nativeFloor(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("floor() takes exactly one argument.");
    }
    return std::floor(asNumber(arguments[0]));
}

auto nativeCeil(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("ceil() takes exactly one argument.");
    }
    return std::ceil(asNumber(arguments[0]));
}

auto nativeRound(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("round() takes exactly one argument.");
    }
    return std::round(asNumber(arguments[0]));
}

auto nativeSin(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("sin() takes exactly one argument.");
    }
    return std::sin(asNumber(arguments[0]));
}

auto nativeCos(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("cos() takes exactly one argument.");
    }
    return std::cos(asNumber(arguments[0]));
}

auto nativeTan(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("tan() takes exactly one argument.");
    }
    return std::tan(asNumber(arguments[0]));
}

auto nativeMin(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.empty()) {
        throw std::runtime_error("min() requires at least one argument.");
    }
    double minVal = asNumber(arguments[0]);
    for (size_t i = 1; i < arguments.size(); ++i) {
        minVal = std::min(minVal, asNumber(arguments[i]));
    }
    return minVal;
}

auto nativeMax(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.empty()) {
        throw std::runtime_error("max() requires at least one argument.");
    }
    double maxVal = asNumber(arguments[0]);
    for (size_t i = 1; i < arguments.size(); ++i) {
        maxVal = std::max(maxVal, asNumber(arguments[i]));
    }
    return maxVal;
}

// ============ std.string functions ============

auto nativeSubstring(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 2 || arguments.size() > 3) {
        throw std::runtime_error("substring() takes 2 or 3 arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("First argument to substring() must be a string.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    size_t start = static_cast<size_t>(asNumber(arguments[1]));
    size_t length = (arguments.size() == 3) 
        ? static_cast<size_t>(asNumber(arguments[2]))
        : str.length() - start;
    
    if (start >= str.length()) {
        return std::string("");
    }
    return str.substr(start, length);
}

auto nativeSplit(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("split() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || 
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to split() must be strings.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    std::string delim = std::get<std::string>(arguments[1]);
    
    auto result = std::make_shared<Array>();
    if (delim.empty()) {
        for (char c : str) {
            result->elements.push_back(std::string(1, c));
        }
        return result;
    }
    
    size_t start = 0;
    size_t end = str.find(delim);
    while (end != std::string::npos) {
        result->elements.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    result->elements.push_back(str.substr(start));
    return result;
}

auto nativeJoin(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("join() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("join() requires an array and a string separator.");
    }
    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    std::string sep = std::get<std::string>(arguments[1]);
    
    std::stringstream ss;
    for (size_t i = 0; i < arr->elements.size(); ++i) {
        if (std::holds_alternative<std::string>(arr->elements[i])) {
            ss << std::get<std::string>(arr->elements[i]);
        } else if (std::holds_alternative<double>(arr->elements[i])) {
            ss << std::get<double>(arr->elements[i]);
        }
        if (i + 1 < arr->elements.size()) {
            ss << sep;
        }
    }
    return ss.str();
}

auto nativeToUpper(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("toUpper() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to toUpper() must be a string.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return str;
}

auto nativeToLower(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("toLower() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to toLower() must be a string.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return str;
}

auto nativeTrim(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("trim() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to trim() must be a string.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    
    // Trim from start
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    
    // Trim from end
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), str.end());
    
    return str;
}

auto nativeReplace(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 3) {
        throw std::runtime_error("replace() takes exactly three arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1]) ||
        !std::holds_alternative<std::string>(arguments[2])) {
        throw std::runtime_error("All arguments to replace() must be strings.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    std::string from = std::get<std::string>(arguments[1]);
    std::string to = std::get<std::string>(arguments[2]);
    
    if (from.empty()) return str;
    
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

auto nativeStartsWith(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("startsWith() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to startsWith() must be strings.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    std::string prefix = std::get<std::string>(arguments[1]);
    return str.rfind(prefix, 0) == 0;
}

auto nativeEndsWith(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("endsWith() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to endsWith() must be strings.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    std::string suffix = std::get<std::string>(arguments[1]);
    if (suffix.length() > str.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

auto nativeIndexOf(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("indexOf() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to indexOf() must be strings.");
    }
    std::string str = std::get<std::string>(arguments[0]);
    std::string substr = std::get<std::string>(arguments[1]);
    size_t pos = str.find(substr);
    if (pos == std::string::npos) {
        return -1.0;
    }
    return static_cast<double>(pos);
}

// ============ std.array functions ============

auto nativeMap(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("map() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("First argument to map() must be an array.");
    }
    if (!std::holds_alternative<std::shared_ptr<Callable>>(arguments[1])) {
        throw std::runtime_error("Second argument to map() must be a function.");
    }
    
    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto func = std::get<std::shared_ptr<Callable>>(arguments[1]);
    auto result = std::make_shared<Array>();
    
    for (const auto& elem : arr->elements) {
        result->elements.push_back(func->call(interp, {elem}));
    }
    return result;
}

auto nativeFilter(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("filter() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("First argument to filter() must be an array.");
    }
    if (!std::holds_alternative<std::shared_ptr<Callable>>(arguments[1])) {
        throw std::runtime_error("Second argument to filter() must be a function.");
    }
    
    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto func = std::get<std::shared_ptr<Callable>>(arguments[1]);
    auto result = std::make_shared<Array>();
    
    for (const auto& elem : arr->elements) {
        Value testResult = func->call(interp, {elem});
        if (isTruthy(testResult)) {
            result->elements.push_back(elem);
        }
    }
    return result;
}

auto nativeReduce(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 2 || arguments.size() > 3) {
        throw std::runtime_error("reduce() takes 2 or 3 arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("First argument to reduce() must be an array.");
    }
    if (!std::holds_alternative<std::shared_ptr<Callable>>(arguments[1])) {
        throw std::runtime_error("Second argument to reduce() must be a function.");
    }
    
    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto func = std::get<std::shared_ptr<Callable>>(arguments[1]);
    
    if (arr->elements.empty()) {
        if (arguments.size() == 3) {
            return arguments[2];
        }
        throw std::runtime_error("reduce() of empty array with no initial value.");
    }
    
    size_t start = 0;
    Value accumulator;
    
    if (arguments.size() == 3) {
        accumulator = arguments[2];
        start = 0;
    } else {
        accumulator = arr->elements[0];
        start = 1;
    }
    
    for (size_t i = start; i < arr->elements.size(); ++i) {
        accumulator = func->call(interp, {accumulator, arr->elements[i]});
    }
    return accumulator;
}

auto nativeSort(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("sort() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("Argument to sort() must be an array.");
    }
    
    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto result = std::make_shared<Array>(*arr);
    
    std::sort(result->elements.begin(), result->elements.end(), 
        [](const Value& a, const Value& b) {
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                return std::get<double>(a) < std::get<double>(b);
            }
            if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
                return std::get<std::string>(a) < std::get<std::string>(b);
            }
            return false;
        });
    return result;
}

auto nativeReverse(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("reverse() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("Argument to reverse() must be an array.");
    }
    
    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto result = std::make_shared<Array>();
    result->elements.assign(arr->elements.rbegin(), arr->elements.rend());
    return result;
}

auto nativeConcat(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("concat() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0]) ||
        !std::holds_alternative<std::shared_ptr<Array>>(arguments[1])) {
        throw std::runtime_error("Both arguments to concat() must be arrays.");
    }
    
    auto arr1 = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto arr2 = std::get<std::shared_ptr<Array>>(arguments[1]);
    auto result = std::make_shared<Array>();
    
    result->elements.insert(result->elements.end(), 
                           arr1->elements.begin(), arr1->elements.end());
    result->elements.insert(result->elements.end(), 
                           arr2->elements.begin(), arr2->elements.end());
    return result;
}

auto nativeSlice(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 2 || arguments.size() > 3) {
        throw std::runtime_error("slice() takes 2 or 3 arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("First argument to slice() must be an array.");
    }
    
    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    size_t start = static_cast<size_t>(asNumber(arguments[1]));
    size_t end = (arguments.size() == 3) 
        ? static_cast<size_t>(asNumber(arguments[2]))
        : arr->elements.size();
    
    auto result = std::make_shared<Array>();
    if (start >= arr->elements.size()) {
        return result;
    }
    end = std::min(end, arr->elements.size());
    
    result->elements.assign(arr->elements.begin() + start, 
                           arr->elements.begin() + end);
    return result;
}

// ============ std.io functions ============

auto nativeReadFile(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("readFile() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to readFile() must be a string.");
    }
    
    std::string filename = std::get<std::string>(arguments[0]);
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto nativeWriteFile(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("writeFile() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to writeFile() must be strings.");
    }
    
    std::string filename = std::get<std::string>(arguments[0]);
    std::string content = std::get<std::string>(arguments[1]);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filename);
    }
    
    file << content;
    return Nil{};
}

auto nativeAppendFile(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("appendFile() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to appendFile() must be strings.");
    }
    
    std::string filename = std::get<std::string>(arguments[0]);
    std::string content = std::get<std::string>(arguments[1]);
    
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for appending: " + filename);
    }
    
    file << content;
    return Nil{};
}

auto nativeFileExists(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("fileExists() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to fileExists() must be a string.");
    }
    
    std::string filename = std::get<std::string>(arguments[0]);
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void registerNativeFunctions(Interpreter& interp) {
    // Core functions
    interp.defineGlobal("print", Value{std::make_shared<NativeFunction>(
        "print", -1, nativePrint)});
    interp.defineGlobal("clock", Value{std::make_shared<NativeFunction>(
        "clock", 0, nativeClock)});
    interp.defineGlobal("len", Value{std::make_shared<NativeFunction>(
        "len", 1, nativeLen)});
    
    // Array functions
    interp.defineGlobal("push", Value{std::make_shared<NativeFunction>(
        "push", 2, nativePush)});
    interp.defineGlobal("pop", Value{std::make_shared<NativeFunction>(
        "pop", 1, nativePop)});
    interp.defineGlobal("shift", Value{std::make_shared<NativeFunction>(
        "shift", 1, nativeShift)});
    interp.defineGlobal("unshift", Value{std::make_shared<NativeFunction>(
        "unshift", 2, nativeUnshift)});
    interp.defineGlobal("splice", Value{std::make_shared<NativeFunction>(
        "splice", -1, nativeSplice)});
    
    // Map functions
    interp.defineGlobal("keys", Value{std::make_shared<NativeFunction>(
        "keys", 1, nativeKeys)});
    interp.defineGlobal("values", Value{std::make_shared<NativeFunction>(
        "values", 1, nativeValues)});
    interp.defineGlobal("hasKey", Value{std::make_shared<NativeFunction>(
        "hasKey", 2, nativeHasKey)});
    interp.defineGlobal("has", Value{std::make_shared<NativeFunction>(
        "has", 2, nativeHas)});
    interp.defineGlobal("delete", Value{std::make_shared<NativeFunction>(
        "delete", 2, nativeDelete)});
    interp.defineGlobal("entries", Value{std::make_shared<NativeFunction>(
        "entries", 1, nativeEntries)});
    
    // Set functions
    interp.defineGlobal("Set", Value{std::make_shared<NativeFunction>(
        "Set", 0, nativeSet)});
    interp.defineGlobal("setAdd", Value{std::make_shared<NativeFunction>(
        "setAdd", 2, nativeSetAdd)});
    interp.defineGlobal("setHas", Value{std::make_shared<NativeFunction>(
        "setHas", 2, nativeSetHas)});
    interp.defineGlobal("setDelete", Value{std::make_shared<NativeFunction>(
        "setDelete", 2, nativeSetDelete)});
    interp.defineGlobal("setSize", Value{std::make_shared<NativeFunction>(
        "setSize", 1, nativeSetSize)});
    
    // std.math functions
    interp.defineGlobal("sqrt", Value{std::make_shared<NativeFunction>(
        "sqrt", 1, nativeSqrt)});
    interp.defineGlobal("pow", Value{std::make_shared<NativeFunction>(
        "pow", 2, nativePow)});
    interp.defineGlobal("abs", Value{std::make_shared<NativeFunction>(
        "abs", 1, nativeAbs)});
    interp.defineGlobal("floor", Value{std::make_shared<NativeFunction>(
        "floor", 1, nativeFloor)});
    interp.defineGlobal("ceil", Value{std::make_shared<NativeFunction>(
        "ceil", 1, nativeCeil)});
    interp.defineGlobal("round", Value{std::make_shared<NativeFunction>(
        "round", 1, nativeRound)});
    interp.defineGlobal("sin", Value{std::make_shared<NativeFunction>(
        "sin", 1, nativeSin)});
    interp.defineGlobal("cos", Value{std::make_shared<NativeFunction>(
        "cos", 1, nativeCos)});
    interp.defineGlobal("tan", Value{std::make_shared<NativeFunction>(
        "tan", 1, nativeTan)});
    interp.defineGlobal("min", Value{std::make_shared<NativeFunction>(
        "min", -1, nativeMin)});
    interp.defineGlobal("max", Value{std::make_shared<NativeFunction>(
        "max", -1, nativeMax)});
    
    // std.string functions
    interp.defineGlobal("substring", Value{std::make_shared<NativeFunction>(
        "substring", -1, nativeSubstring)});
    interp.defineGlobal("split", Value{std::make_shared<NativeFunction>(
        "split", 2, nativeSplit)});
    interp.defineGlobal("join", Value{std::make_shared<NativeFunction>(
        "join", 2, nativeJoin)});
    interp.defineGlobal("toUpper", Value{std::make_shared<NativeFunction>(
        "toUpper", 1, nativeToUpper)});
    interp.defineGlobal("toLower", Value{std::make_shared<NativeFunction>(
        "toLower", 1, nativeToLower)});
    interp.defineGlobal("trim", Value{std::make_shared<NativeFunction>(
        "trim", 1, nativeTrim)});
    interp.defineGlobal("replace", Value{std::make_shared<NativeFunction>(
        "replace", 3, nativeReplace)});
    interp.defineGlobal("startsWith", Value{std::make_shared<NativeFunction>(
        "startsWith", 2, nativeStartsWith)});
    interp.defineGlobal("endsWith", Value{std::make_shared<NativeFunction>(
        "endsWith", 2, nativeEndsWith)});
    interp.defineGlobal("indexOf", Value{std::make_shared<NativeFunction>(
        "indexOf", 2, nativeIndexOf)});
    
    // std.array functions
    interp.defineGlobal("map", Value{std::make_shared<NativeFunction>(
        "map", 2, nativeMap)});
    interp.defineGlobal("filter", Value{std::make_shared<NativeFunction>(
        "filter", 2, nativeFilter)});
    interp.defineGlobal("reduce", Value{std::make_shared<NativeFunction>(
        "reduce", -1, nativeReduce)});
    interp.defineGlobal("sort", Value{std::make_shared<NativeFunction>(
        "sort", 1, nativeSort)});
    interp.defineGlobal("reverse", Value{std::make_shared<NativeFunction>(
        "reverse", 1, nativeReverse)});
    interp.defineGlobal("concat", Value{std::make_shared<NativeFunction>(
        "concat", 2, nativeConcat)});
    interp.defineGlobal("slice", Value{std::make_shared<NativeFunction>(
        "slice", -1, nativeSlice)});
    
    // std.io functions
    interp.defineGlobal("readFile", Value{std::make_shared<NativeFunction>(
        "readFile", 1, nativeReadFile)});
    interp.defineGlobal("writeFile", Value{std::make_shared<NativeFunction>(
        "writeFile", 2, nativeWriteFile)});
    interp.defineGlobal("appendFile", Value{std::make_shared<NativeFunction>(
        "appendFile", 2, nativeAppendFile)});
    interp.defineGlobal("fileExists", Value{std::make_shared<NativeFunction>(
        "fileExists", 1, nativeFileExists)});
}

}
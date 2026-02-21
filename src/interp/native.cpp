#include "native.hpp"
#include "interpreter.hpp"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <thread>
#include <regex>
#include <ctime>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#ifndef _WIN32
#include <fcntl.h>
#include <sys/types.h>
#endif
#include <mutex>
#include <unordered_map>

namespace izi {
auto nativePrint(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    for (size_t i = 0; i < arguments.size(); ++i) {
        printValue(arguments[i]);
        if (i + 1 < arguments.size()) std::cout << " ";
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

auto nativeStr(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("str() takes exactly one argument.");
    }
    return valueToString(arguments[0]);
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
        throw std::runtime_error("setAdd() only supports primitive types (string, number, boolean, nil), but got: " +
                                 getTypeName(valueVal));
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

auto nativeClamp(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 3) {
        throw std::runtime_error("clamp() takes exactly three arguments.");
    }
    double x = asNumber(arguments[0]);
    double minVal = asNumber(arguments[1]);
    double maxVal = asNumber(arguments[2]);
    if (minVal > maxVal) {
        throw std::runtime_error("clamp() min must be less than or equal to max.");
    }
    return std::max(minVal, std::min(maxVal, x));
}

auto nativeSign(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("sign() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    if (std::isnan(val)) {
        return val;  // NaN
    }
    if (val > 0) return 1.0;
    if (val < 0) return -1.0;
    return 0.0;
}

auto nativeIsFinite(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("isFinite() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    return std::isfinite(val);
}

auto nativeIsNaN(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("isNaN() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    return std::isnan(val);
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
    size_t length = (arguments.size() == 3) ? static_cast<size_t>(asNumber(arguments[2])) : str.length() - start;

    if (start >= str.length()) {
        return std::string("");
    }
    return str.substr(start, length);
}

auto nativeSplit(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("split() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
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
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::toupper(c); });
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
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
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
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); }));

    // Trim from end
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
              str.end());

    return str;
}

auto nativeReplace(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 3) {
        throw std::runtime_error("replace() takes exactly three arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1]) ||
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
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
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
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
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
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
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

    std::sort(result->elements.begin(), result->elements.end(), [](const Value& a, const Value& b) {
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

    result->elements.insert(result->elements.end(), arr1->elements.begin(), arr1->elements.end());
    result->elements.insert(result->elements.end(), arr2->elements.begin(), arr2->elements.end());
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
    size_t end = (arguments.size() == 3) ? static_cast<size_t>(asNumber(arguments[2])) : arr->elements.size();

    auto result = std::make_shared<Array>();
    if (start >= arr->elements.size()) {
        return result;
    }
    end = std::min(end, arr->elements.size());

    result->elements.assign(arr->elements.begin() + start, arr->elements.begin() + end);
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
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
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
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
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

// ============ std.log functions ============

auto nativeLogInfo(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.info() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to log.info() must be a string.");
    }
    std::cout << "[INFO] " << std::get<std::string>(arguments[0]) << "\n";
    return Nil{};
}

auto nativeLogWarn(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.warn() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to log.warn() must be a string.");
    }
    std::cout << "[WARN] " << std::get<std::string>(arguments[0]) << "\n";
    return Nil{};
}

auto nativeLogError(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.error() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to log.error() must be a string.");
    }
    std::cerr << "[ERROR] " << std::get<std::string>(arguments[0]) << "\n";
    return Nil{};
}

auto nativeLogDebug(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.debug() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to log.debug() must be a string.");
    }
    std::cout << "[DEBUG] " << std::get<std::string>(arguments[0]) << "\n";
    return Nil{};
}

// std.assert functions
auto nativeAssertOk(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 1 || arguments.size() > 2) {
        throw std::runtime_error("assert.ok() takes 1 or 2 arguments.");
    }

    bool condition = isTruthy(arguments[0]);

    if (!condition) {
        std::string message = "Assertion failed";
        if (arguments.size() == 2) {
            if (!std::holds_alternative<std::string>(arguments[1])) {
                throw std::runtime_error("Second argument to assert.ok() must be a string.");
            }
            message = std::get<std::string>(arguments[1]);
        }
        throw std::runtime_error(message);
    }

    return Nil{};
}

auto nativeAssertEq(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("assert.eq() takes exactly 2 arguments.");
    }

    if (arguments[0] != arguments[1]) {
        std::ostringstream oss;
        oss << "Assertion failed: expected values to be equal, but got ";
        oss << valueToString(arguments[0]);
        oss << " and ";
        oss << valueToString(arguments[1]);
        throw std::runtime_error(oss.str());
    }

    return Nil{};
}

auto nativeAssertNe(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("assert.ne() takes exactly 2 arguments.");
    }

    if (arguments[0] == arguments[1]) {
        std::ostringstream oss;
        oss << "Assertion failed: expected values to be different, but both were ";
        oss << valueToString(arguments[0]);
        throw std::runtime_error(oss.str());
    }

    return Nil{};
}

// ============ std.env functions ============

auto nativeEnvGet(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("env.get() takes exactly one argument.");
    }

    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("env.get() argument must be a string.");
    }

    const std::string& name = std::get<std::string>(arguments[0]);
    const char* value = std::getenv(name.c_str());

    if (value == nullptr) {
        return Nil{};
    }

    return std::string(value);
}

auto nativeEnvSet(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("env.set() takes exactly two arguments.");
    }

    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("env.set() first argument must be a string.");
    }

    if (!std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("env.set() second argument must be a string.");
    }

    const std::string& name = std::get<std::string>(arguments[0]);
    const std::string& value = std::get<std::string>(arguments[1]);

    // Use setenv for POSIX systems (Linux, macOS)
    int result;
#ifdef _WIN32
    result = _putenv_s(name.c_str(), value.c_str());
#else
    result = setenv(name.c_str(), value.c_str(), 1);
#endif

    if (result != 0) {
        throw std::runtime_error("env.set() failed to set environment variable '" + name + "'");
    }

    return Nil{};
}

auto nativeEnvExists(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("env.exists() takes exactly one argument.");
    }

    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("env.exists() argument must be a string.");
    }

    const std::string& name = std::get<std::string>(arguments[0]);
    const char* value = std::getenv(name.c_str());

    return value != nullptr;
}

// std.process functions
auto nativeProcessExit(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("process.exit() takes exactly one argument.");
    }

    if (!std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("process.exit() argument must be a number.");
    }

    int exitCode = static_cast<int>(std::get<double>(arguments[0]));
    std::exit(exitCode);

    // This line will never be reached, but is here to satisfy the compiler
    return Nil{};
}

auto nativeProcessStatus(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (!arguments.empty()) {
        throw std::runtime_error("process.status() takes no arguments.");
    }

    // Return 0 as the status code (running normally)
    // In a real implementation, this could track the last exit code or process state
    return 0.0;
}

auto nativeProcessArgs(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (!arguments.empty()) {
        throw std::runtime_error("process.args() takes no arguments.");
    }

    const auto& cmdArgs = interp.getCommandLineArgs();
    auto argsArray = std::make_shared<Array>();

    for (const auto& arg : cmdArgs) {
        argsArray->elements.push_back(arg);
    }

    return argsArray;
}

// std.path functions
auto nativePathJoin(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.empty()) {
        return std::string("");
    }

    std::string result;

    for (size_t i = 0; i < arguments.size(); ++i) {
        if (!std::holds_alternative<std::string>(arguments[i])) {
            throw std::runtime_error("path.join() requires all arguments to be strings.");
        }

        std::string part = std::get<std::string>(arguments[i]);

        // Skip empty parts
        if (part.empty()) {
            continue;
        }

        // Add separator if needed
        if (!result.empty() && result.back() != '/') {
            result += '/';
        }

        // Remove leading slashes from parts (except the first absolute path)
        if (!result.empty() && !part.empty() && part[0] == '/') {
            part = part.substr(1);
        }

        result += part;
    }

    return result;
}

auto nativePathBasename(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("path.basename() takes exactly one argument.");
    }

    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("path.basename() requires a string argument.");
    }

    std::string path = std::get<std::string>(arguments[0]);

    // Remove trailing slashes
    while (!path.empty() && path.back() == '/') {
        path.pop_back();
    }

    // If path is empty after removing slashes, return "/"
    if (path.empty()) {
        return std::string("/");
    }

    // Find the last slash
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return path;
    }

    return path.substr(pos + 1);
}

auto nativePathDirname(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("path.dirname() takes exactly one argument.");
    }

    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("path.dirname() requires a string argument.");
    }

    std::string path = std::get<std::string>(arguments[0]);

    // Remove trailing slashes
    while (!path.empty() && path.back() == '/') {
        path.pop_back();
    }

    // If path is empty or no slash, return "."
    if (path.empty()) {
        return std::string(".");
    }

    // Find the last slash
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return std::string(".");
    }

    // If slash is at the beginning, return "/"
    if (pos == 0) {
        return std::string("/");
    }

    return path.substr(0, pos);
}

auto nativePathExtname(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("path.extname() takes exactly one argument.");
    }

    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("path.extname() requires a string argument.");
    }

    std::string path = std::get<std::string>(arguments[0]);

    // Get the basename first
    size_t slashPos = path.find_last_of('/');
    std::string basename = (slashPos == std::string::npos) ? path : path.substr(slashPos + 1);

    // Find the last dot in the basename
    size_t dotPos = basename.find_last_of('.');

    // No extension if no dot, or if dot is at the beginning (hidden file)
    if (dotPos == std::string::npos || dotPos == 0) {
        return std::string("");
    }

    return basename.substr(dotPos);
}

auto nativePathNormalize(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("path.normalize() takes exactly one argument.");
    }

    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("path.normalize() requires a string argument.");
    }

    std::string path = std::get<std::string>(arguments[0]);

    // Handle empty path
    if (path.empty()) {
        return std::string(".");
    }

    bool isAbsolute = (!path.empty() && path[0] == '/');

    // Split path into parts
    std::vector<std::string> parts;
    std::string current;

    for (char c : path) {
        if (c == '/') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }

    // Process parts, handling . and ..
    std::vector<std::string> stack;

    for (const auto& part : parts) {
        if (part == "..") {
            // Go up one directory if possible
            if (!stack.empty() && stack.back() != "..") {
                stack.pop_back();
            } else if (!isAbsolute) {
                // For relative paths, keep .. if we can't go further up
                stack.push_back(part);
            }
            // For absolute paths, ignore .. at root
        } else if (part != ".") {
            // Add normal parts (skip ".")
            stack.push_back(part);
        }
    }

    // Build result
    std::string result;
    if (isAbsolute) {
        result = "/";
    }

    for (size_t i = 0; i < stack.size(); ++i) {
        result += stack[i];
        if (i + 1 < stack.size()) {
            result += "/";
        }
    }

    // If result is empty, return "." for current directory
    if (result.empty()) {
        return std::string(".");
    }

    return result;
}

// ============ std.fs functions ============

auto nativeFsExists(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("fs.exists() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to fs.exists() must be a string.");
    }

    std::string path = std::get<std::string>(arguments[0]);
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

auto nativeFsRead(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("fs.read() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to fs.read() must be a string.");
    }

    std::string path = std::get<std::string>(arguments[0]);
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for reading: " + path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto nativeFsWrite(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("fs.write() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to fs.write() must be strings.");
    }

    std::string path = std::get<std::string>(arguments[0]);
    std::string content = std::get<std::string>(arguments[1]);

    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }

    file << content;
    if (file.fail()) {
        throw std::runtime_error("Failed to write to file: " + path);
    }

    return Nil{};
}

auto nativeFsAppend(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("fs.append() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to fs.append() must be strings.");
    }

    std::string path = std::get<std::string>(arguments[0]);
    std::string content = std::get<std::string>(arguments[1]);

    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for appending: " + path);
    }

    file << content;
    if (file.fail()) {
        throw std::runtime_error("Failed to append to file: " + path);
    }

    return Nil{};
}

auto nativeFsRemove(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("fs.remove() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to fs.remove() must be a string.");
    }

    std::string path = std::get<std::string>(arguments[0]);

    if (std::remove(path.c_str()) != 0) {
        throw std::runtime_error("Failed to remove file: " + path);
    }

    return Nil{};
}

// std.time functions
auto nativeTimeNow(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 0) {
        throw std::runtime_error("time.now() takes no arguments.");
    }

    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return static_cast<double>(ms) / 1000.0;
}

auto nativeTimeSleep(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("time.sleep() takes exactly one argument.");
    }
    if (!std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("Argument to time.sleep() must be a number (seconds).");
    }

    double seconds = std::get<double>(arguments[0]);
    if (seconds < 0) {
        throw std::runtime_error("time.sleep() argument must be non-negative.");
    }

    auto duration = std::chrono::milliseconds(static_cast<long long>(seconds * 1000));
    std::this_thread::sleep_for(duration);

    return Nil{};
}

auto nativeTimeFormat(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 1 || arguments.size() > 2) {
        throw std::runtime_error("time.format() takes 1 or 2 arguments.");
    }
    if (!std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("First argument to time.format() must be a number (timestamp).");
    }

    double timestamp = std::get<double>(arguments[0]);
    std::string format = "%Y-%m-%d %H:%M:%S";

    if (arguments.size() == 2) {
        if (!std::holds_alternative<std::string>(arguments[1])) {
            throw std::runtime_error("Second argument to time.format() must be a string (format).");
        }
        format = std::get<std::string>(arguments[1]);
    }

    std::time_t time = static_cast<std::time_t>(timestamp);
    std::tm* tm_info = std::localtime(&time);

    char buffer[256];
    std::strftime(buffer, sizeof(buffer), format.c_str(), tm_info);

    return std::string(buffer);
}

// std.json functions
static Value jsonValueFromString(const std::string& jsonStr, size_t& pos);

static void skipWhitespace(const std::string& str, size_t& pos) {
    while (pos < str.size() && std::isspace(str[pos])) {
        ++pos;
    }
}

static Value parseJsonNull(const std::string& str, size_t& pos) {
    if (str.substr(pos, 4) == "null") {
        pos += 4;
        return Nil{};
    }
    throw std::runtime_error("Invalid JSON: expected 'null'");
}

static Value parseJsonBool(const std::string& str, size_t& pos) {
    if (str.substr(pos, 4) == "true") {
        pos += 4;
        return true;
    }
    if (str.substr(pos, 5) == "false") {
        pos += 5;
        return false;
    }
    throw std::runtime_error("Invalid JSON: expected boolean");
}

static Value parseJsonNumber(const std::string& str, size_t& pos) {
    size_t start = pos;
    if (str[pos] == '-') ++pos;

    while (pos < str.size() && std::isdigit(str[pos])) ++pos;

    if (pos < str.size() && str[pos] == '.') {
        ++pos;
        while (pos < str.size() && std::isdigit(str[pos])) ++pos;
    }

    if (pos < str.size() && (str[pos] == 'e' || str[pos] == 'E')) {
        ++pos;
        if (pos < str.size() && (str[pos] == '+' || str[pos] == '-')) ++pos;
        while (pos < str.size() && std::isdigit(str[pos])) ++pos;
    }

    return std::stod(str.substr(start, pos - start));
}

static Value parseJsonString(const std::string& str, size_t& pos) {
    if (str[pos] != '"') {
        throw std::runtime_error("Invalid JSON: expected string");
    }
    ++pos;

    std::string result;
    while (pos < str.size() && str[pos] != '"') {
        if (str[pos] == '\\') {
            ++pos;
            if (pos >= str.size()) {
                throw std::runtime_error("Invalid JSON: unterminated string escape");
            }
            switch (str[pos]) {
                case '"':
                    result += '"';
                    break;
                case '\\':
                    result += '\\';
                    break;
                case '/':
                    result += '/';
                    break;
                case 'b':
                    result += '\b';
                    break;
                case 'f':
                    result += '\f';
                    break;
                case 'n':
                    result += '\n';
                    break;
                case 'r':
                    result += '\r';
                    break;
                case 't':
                    result += '\t';
                    break;
                default:
                    throw std::runtime_error("Invalid JSON: unknown escape sequence");
            }
        } else {
            result += str[pos];
        }
        ++pos;
    }

    if (pos >= str.size()) {
        throw std::runtime_error("Invalid JSON: unterminated string");
    }
    ++pos;  // skip closing quote

    return result;
}

static Value parseJsonArray(const std::string& str, size_t& pos) {
    if (str[pos] != '[') {
        throw std::runtime_error("Invalid JSON: expected array");
    }
    ++pos;

    auto arr = std::make_shared<Array>();
    skipWhitespace(str, pos);

    if (pos < str.size() && str[pos] == ']') {
        ++pos;
        return arr;
    }

    while (pos < str.size()) {
        arr->elements.push_back(jsonValueFromString(str, pos));
        skipWhitespace(str, pos);

        if (pos >= str.size()) {
            throw std::runtime_error("Invalid JSON: unterminated array");
        }

        if (str[pos] == ']') {
            ++pos;
            return arr;
        }

        if (str[pos] != ',') {
            throw std::runtime_error("Invalid JSON: expected ',' or ']' in array");
        }
        ++pos;
        skipWhitespace(str, pos);
    }

    throw std::runtime_error("Invalid JSON: unterminated array");
}

static Value parseJsonObject(const std::string& str, size_t& pos) {
    if (str[pos] != '{') {
        throw std::runtime_error("Invalid JSON: expected object");
    }
    ++pos;

    auto map = std::make_shared<Map>();
    skipWhitespace(str, pos);

    if (pos < str.size() && str[pos] == '}') {
        ++pos;
        return map;
    }

    while (pos < str.size()) {
        skipWhitespace(str, pos);

        if (str[pos] != '"') {
            throw std::runtime_error("Invalid JSON: expected string key in object");
        }

        Value keyVal = parseJsonString(str, pos);
        std::string key = std::get<std::string>(keyVal);

        skipWhitespace(str, pos);
        if (pos >= str.size() || str[pos] != ':') {
            throw std::runtime_error("Invalid JSON: expected ':' after object key");
        }
        ++pos;
        skipWhitespace(str, pos);

        Value value = jsonValueFromString(str, pos);
        map->entries[key] = value;

        skipWhitespace(str, pos);

        if (pos >= str.size()) {
            throw std::runtime_error("Invalid JSON: unterminated object");
        }

        if (str[pos] == '}') {
            ++pos;
            return map;
        }

        if (str[pos] != ',') {
            throw std::runtime_error("Invalid JSON: expected ',' or '}' in object");
        }
        ++pos;
    }

    throw std::runtime_error("Invalid JSON: unterminated object");
}

static Value jsonValueFromString(const std::string& jsonStr, size_t& pos) {
    skipWhitespace(jsonStr, pos);

    if (pos >= jsonStr.size()) {
        throw std::runtime_error("Invalid JSON: unexpected end of input");
    }

    char c = jsonStr[pos];

    if (c == 'n') return parseJsonNull(jsonStr, pos);
    if (c == 't' || c == 'f') return parseJsonBool(jsonStr, pos);
    if (c == '"') return parseJsonString(jsonStr, pos);
    if (c == '[') return parseJsonArray(jsonStr, pos);
    if (c == '{') return parseJsonObject(jsonStr, pos);
    if (c == '-' || std::isdigit(c)) return parseJsonNumber(jsonStr, pos);

    throw std::runtime_error("Invalid JSON: unexpected character");
}

static std::string valueToJson(const Value& value) {
    if (std::holds_alternative<Nil>(value)) {
        return "null";
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    } else if (std::holds_alternative<double>(value)) {
        double d = std::get<double>(value);
        if (std::isnan(d) || std::isinf(d)) {
            return "null";
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(10) << d;
        std::string str = oss.str();
        // Remove trailing zeros
        auto pos = str.find_last_not_of('0');
        if (pos != std::string::npos) {
            str.erase(pos + 1, std::string::npos);
            if (!str.empty() && str.back() == '.') {
                str.pop_back();
            }
        }
        return str;
    } else if (std::holds_alternative<std::string>(value)) {
        std::string str = std::get<std::string>(value);
        std::ostringstream oss;
        oss << '"';
        for (char c : str) {
            switch (c) {
                case '"':
                    oss << "\\\"";
                    break;
                case '\\':
                    oss << "\\\\";
                    break;
                case '\b':
                    oss << "\\b";
                    break;
                case '\f':
                    oss << "\\f";
                    break;
                case '\n':
                    oss << "\\n";
                    break;
                case '\r':
                    oss << "\\r";
                    break;
                case '\t':
                    oss << "\\t";
                    break;
                default:
                    oss << c;
                    break;
            }
        }
        oss << '"';
        return oss.str();
    } else if (std::holds_alternative<std::shared_ptr<Array>>(value)) {
        auto arr = std::get<std::shared_ptr<Array>>(value);
        std::ostringstream oss;
        oss << '[';
        for (size_t i = 0; i < arr->elements.size(); ++i) {
            if (i > 0) oss << ',';
            oss << valueToJson(arr->elements[i]);
        }
        oss << ']';
        return oss.str();
    } else if (std::holds_alternative<std::shared_ptr<Map>>(value)) {
        auto map = std::get<std::shared_ptr<Map>>(value);
        std::ostringstream oss;
        oss << '{';
        bool first = true;
        for (const auto& [key, val] : map->entries) {
            if (!first) oss << ',';
            first = false;
            oss << '"' << key << "\":" << valueToJson(val);
        }
        oss << '}';
        return oss.str();
    }

    return "null";
}

auto nativeJsonParse(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("json.parse() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to json.parse() must be a string.");
    }

    std::string jsonStr = std::get<std::string>(arguments[0]);
    size_t pos = 0;

    try {
        Value result = jsonValueFromString(jsonStr, pos);
        skipWhitespace(jsonStr, pos);
        if (pos < jsonStr.size()) {
            throw std::runtime_error("Invalid JSON: unexpected characters after value");
        }
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }
}

auto nativeJsonStringify(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("json.stringify() takes exactly one argument.");
    }

    try {
        return valueToJson(arguments[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("JSON stringify error: ") + e.what());
    }
}

// std.regex functions
auto nativeRegexMatch(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("regex.match() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to regex.match() must be strings.");
    }

    // NOTE: regex.match() is currently disabled due to a memory issue
    // Use regex.test() and regex.replace() for now
    throw std::runtime_error("regex.match() is currently disabled. Use regex.test() or regex.replace() instead.");
}

auto nativeRegexReplace(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 3) {
        throw std::runtime_error("regex.replace() takes exactly three arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1]) ||
        !std::holds_alternative<std::string>(arguments[2])) {
        throw std::runtime_error("All arguments to regex.replace() must be strings.");
    }

    std::string text = std::get<std::string>(arguments[0]);
    std::string pattern = std::get<std::string>(arguments[1]);
    std::string replacement = std::get<std::string>(arguments[2]);

    try {
        std::regex re(pattern);
        return std::regex_replace(text, re, replacement);
    } catch (const std::regex_error& e) {
        throw std::runtime_error(std::string("Regex error: ") + e.what());
    }
}

auto nativeRegexTest(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2) {
        throw std::runtime_error("regex.test() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to regex.test() must be strings.");
    }

    std::string text = std::get<std::string>(arguments[0]);
    std::string pattern = std::get<std::string>(arguments[1]);

    try {
        std::regex re(pattern);
        return std::regex_search(text, re);
    } catch (const std::regex_error& e) {
        throw std::runtime_error(std::string("Regex error: ") + e.what());
    }
}

// std.http helpers

struct HttpParsedUrl {
    std::string scheme;
    std::string host;
    int port;
    std::string path;
};

static HttpParsedUrl httpParseUrl(const std::string& url) {
    HttpParsedUrl result;
    size_t schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) {
        throw std::runtime_error("http: invalid URL, missing scheme: " + url);
    }
    result.scheme = url.substr(0, schemeEnd);
    std::string rest = url.substr(schemeEnd + 3);

    size_t pathStart = rest.find('/');
    std::string hostPort;
    if (pathStart == std::string::npos) {
        hostPort = rest;
        result.path = "/";
    } else {
        hostPort = rest.substr(0, pathStart);
        result.path = rest.substr(pathStart);
    }

    size_t colonPos = hostPort.find(':');
    if (colonPos != std::string::npos) {
        result.host = hostPort.substr(0, colonPos);
        result.port = std::stoi(hostPort.substr(colonPos + 1));
    } else {
        result.host = hostPort;
        result.port = (result.scheme == "https") ? 443 : 80;
    }

    if (result.host.empty()) {
        throw std::runtime_error("http: invalid URL, empty host: " + url);
    }
    return result;
}

static std::string httpSendRequest(const std::string& method, const std::string& url, const std::string& body,
                                   const std::string& contentType) {
    HttpParsedUrl parsed = httpParseUrl(url);

    if (parsed.scheme == "https") {
        throw std::runtime_error("http: HTTPS is not supported in this version. Use HTTP.");
    }
    if (parsed.scheme != "http") {
        throw std::runtime_error("http: unsupported scheme '" + parsed.scheme + "'. Only HTTP is supported.");
    }

    struct addrinfo hints {
    }, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    std::string portStr = std::to_string(parsed.port);
    int rc = getaddrinfo(parsed.host.c_str(), portStr.c_str(), &hints, &res);
    if (rc != 0) {
        throw std::runtime_error("http: failed to resolve host '" + parsed.host +
                                 "': " + std::string(gai_strerror(rc)));
    }

    int sock = -1;
    for (struct addrinfo* rp = res; rp != nullptr; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) continue;
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);
    if (sock < 0) {
        throw std::runtime_error("http: failed to connect to '" + parsed.host + ":" + portStr + "'");
    }

    std::ostringstream req;
    req << method << " " << parsed.path << " HTTP/1.1\r\n";
    req << "Host: " << parsed.host << "\r\n";
    req << "User-Agent: IziLang/0.3\r\n";
    req << "Accept: */*\r\n";
    req << "Connection: close\r\n";
    if (!body.empty()) {
        req << "Content-Type: " << contentType << "\r\n";
        req << "Content-Length: " << body.size() << "\r\n";
    }
    req << "\r\n";
    if (!body.empty()) {
        req << body;
    }

    std::string reqStr = req.str();
    size_t sent = 0;
    while (sent < reqStr.size()) {
        ssize_t n = send(sock, reqStr.c_str() + sent, reqStr.size() - sent, 0);
        if (n < 0) {
            close(sock);
            throw std::runtime_error("http: error sending request");
        }
        sent += static_cast<size_t>(n);
    }

    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response.append(buffer, static_cast<size_t>(n));
    }
    close(sock);
    return response;
}

static Value httpParseResponse(const std::string& response) {
    auto result = std::make_shared<Map>();

    size_t headerEnd = response.find("\r\n\r\n");
    std::string headerSection;
    std::string body;
    if (headerEnd != std::string::npos) {
        headerSection = response.substr(0, headerEnd);
        body = response.substr(headerEnd + 4);
    } else {
        headerSection = response;
    }

    size_t firstLineEnd = headerSection.find("\r\n");
    std::string statusLine = headerSection.substr(0, firstLineEnd);
    int statusCode = 0;
    std::string statusText;
    size_t sp1 = statusLine.find(' ');
    if (sp1 != std::string::npos) {
        size_t sp2 = statusLine.find(' ', sp1 + 1);
        std::string codeStr =
            (sp2 != std::string::npos) ? statusLine.substr(sp1 + 1, sp2 - sp1 - 1) : statusLine.substr(sp1 + 1);
        try {
            statusCode = std::stoi(codeStr);
        } catch (const std::exception&) {
            // Malformed status line; statusCode remains 0 (unknown)
        }
        if (sp2 != std::string::npos) {
            statusText = statusLine.substr(sp2 + 1);
        }
    }

    auto headersMap = std::make_shared<Map>();
    size_t pos = (firstLineEnd != std::string::npos) ? firstLineEnd + 2 : headerSection.size();
    while (pos < headerSection.size()) {
        size_t lineEnd = headerSection.find("\r\n", pos);
        if (lineEnd == std::string::npos) lineEnd = headerSection.size();
        std::string line = headerSection.substr(pos, lineEnd - pos);
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string val = line.substr(colonPos + 1);
            size_t valStart = val.find_first_not_of(' ');
            if (valStart != std::string::npos) val = val.substr(valStart);
            headersMap->entries[key] = val;
        }
        pos = lineEnd + 2;
    }

    result->entries["status"] = static_cast<double>(statusCode);
    result->entries["statusText"] = statusText;
    result->entries["body"] = body;
    result->entries["headers"] = Value{headersMap};
    result->entries["ok"] = static_cast<bool>(statusCode >= 200 && statusCode < 300);
    return Value{result};
}

// std.http functions
auto nativeHttpGet(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("http.get() requires a URL string argument.");
    }
    const std::string& url = std::get<std::string>(arguments[0]);
    std::string response = httpSendRequest("GET", url, "", "");
    return httpParseResponse(response);
}

auto nativeHttpPost(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 2 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("http.post() requires a URL string and body argument.");
    }
    const std::string& url = std::get<std::string>(arguments[0]);
    std::string body = valueToString(arguments[1]);
    std::string contentType = "application/x-www-form-urlencoded";
    if (arguments.size() >= 3 && std::holds_alternative<std::string>(arguments[2])) {
        contentType = std::get<std::string>(arguments[2]);
    }
    std::string response = httpSendRequest("POST", url, body, contentType);
    return httpParseResponse(response);
}

auto nativeHttpRequest(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() < 1 || !std::holds_alternative<std::shared_ptr<Map>>(arguments[0])) {
        throw std::runtime_error("http.request() requires an options map argument.");
    }
    auto options = std::get<std::shared_ptr<Map>>(arguments[0]);

    std::string method = "GET";
    std::string url;
    std::string body;
    std::string contentType = "application/x-www-form-urlencoded";

    auto it = options->entries.find("method");
    if (it != options->entries.end()) {
        method = valueToString(it->second);
        std::transform(method.begin(), method.end(), method.begin(), ::toupper);
    }

    it = options->entries.find("url");
    if (it == options->entries.end() || !std::holds_alternative<std::string>(it->second)) {
        throw std::runtime_error("http.request() options map must contain a 'url' string.");
    }
    url = std::get<std::string>(it->second);

    it = options->entries.find("body");
    if (it != options->entries.end()) {
        body = valueToString(it->second);
    }

    it = options->entries.find("contentType");
    if (it != options->entries.end()) {
        contentType = valueToString(it->second);
    }

    std::string response = httpSendRequest(method, url, body, contentType);
    return httpParseResponse(response);
}

// Concurrency: spawn creates a Task from a callable (runs lazily when awaited)
auto nativeSpawn(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("spawn() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::shared_ptr<Callable>>(arguments[0])) {
        throw std::runtime_error("spawn() argument must be a callable.");
    }
    auto task = std::make_shared<Task>();
    task->callable = std::get<std::shared_ptr<Callable>>(arguments[0]);
    task->state = Task::State::Pending;
    return task;
}

// Concurrency: await runs a pending task and returns its result
auto nativeAwait(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1) {
        throw std::runtime_error("await() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::shared_ptr<Task>>(arguments[0])) {
        throw std::runtime_error("await() argument must be a task.");
    }
    auto task = std::get<std::shared_ptr<Task>>(arguments[0]);
    if (task->state == Task::State::Running) {
        throw std::runtime_error("await() called re-entrantly on an already running task.");
    }
    if (task->state == Task::State::Pending) {
        task->state = Task::State::Running;
        try {
            task->result = task->callable->call(interp, {});
            task->state = Task::State::Completed;
        } catch (const std::exception& e) {
            task->state = Task::State::Failed;
            task->errorMessage = e.what();
        }
    }
    if (task->state == Task::State::Failed) {
        throw std::runtime_error("Task failed: " + task->errorMessage);
    }
    return task->result;
}

// ============ std.ipc functions ============

namespace {

struct IpcHandle {
    int fd;
    bool is_read;
    std::string path;
};

static std::mutex ipcMutex;
static std::unordered_map<int, IpcHandle> ipcHandleMap;
static int nextIpcHandle = 1;

static std::string ipcPipePath(const std::string& name) {
    return "/tmp/izi_ipc_" + name;
}

static int ipcAllocHandle(int fd, bool is_read, const std::string& path) {
    std::lock_guard<std::mutex> lock(ipcMutex);
    int id = nextIpcHandle++;
    ipcHandleMap[id] = {fd, is_read, path};
    return id;
}

static IpcHandle ipcGetHandle(int id) {
    std::lock_guard<std::mutex> lock(ipcMutex);
    auto it = ipcHandleMap.find(id);
    if (it == ipcHandleMap.end()) {
        throw std::runtime_error("Invalid IPC handle: " + std::to_string(id));
    }
    return it->second;
}

static void ipcFreeHandle(int id) {
    std::lock_guard<std::mutex> lock(ipcMutex);
    ipcHandleMap.erase(id);
}

}  // anonymous namespace

// ipc.createPipe(name) - creates a named pipe (FIFO) for IPC
auto nativeIpcCreatePipe(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.createPipe() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.createPipe() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = ipcPipePath(name);
    if (mkfifo(path.c_str(), 0600) != 0 && errno != EEXIST) {
        throw std::runtime_error("ipc.createPipe() failed to create pipe '" + name + "': " + strerror(errno));
    }
    return true;
#endif
}

// ipc.openRead(name) - opens a named pipe for reading; returns a handle number
auto nativeIpcOpenRead(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.openRead() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.openRead() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = ipcPipePath(name);
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error("ipc.openRead() failed to open pipe '" + name + "': " + strerror(errno));
    }
    // Remove O_NONBLOCK for blocking reads by default
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    int handle = ipcAllocHandle(fd, true, path);
    return static_cast<double>(handle);
#endif
}

// ipc.openWrite(name) - opens a named pipe for writing; returns a handle number
auto nativeIpcOpenWrite(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.openWrite() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.openWrite() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = ipcPipePath(name);
    int fd = open(path.c_str(), O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error("ipc.openWrite() failed to open pipe '" + name +
                                 "': " + strerror(errno) +
                                 " (ensure a reader has opened the pipe first)");
    }
    // Remove O_NONBLOCK for blocking writes by default
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    int handle = ipcAllocHandle(fd, false, path);
    return static_cast<double>(handle);
#endif
}

// ipc.send(handle, message) - sends a length-prefixed message through a pipe handle
auto nativeIpcSend(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2 || !std::holds_alternative<double>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("ipc.send() takes a handle (number) and a message (string).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.send() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    const std::string& message = std::get<std::string>(arguments[1]);
    IpcHandle h = ipcGetHandle(handle);
    if (h.is_read) {
        throw std::runtime_error("ipc.send() called on a read-only handle.");
    }
    uint32_t len = static_cast<uint32_t>(message.size());
    uint8_t lenBuf[4] = {
        static_cast<uint8_t>((len >> 24) & 0xFF),
        static_cast<uint8_t>((len >> 16) & 0xFF),
        static_cast<uint8_t>((len >> 8) & 0xFF),
        static_cast<uint8_t>(len & 0xFF)
    };
    if (write(h.fd, lenBuf, 4) != 4) {
        throw std::runtime_error("ipc.send() failed to write message length: " + std::string(strerror(errno)));
    }
    ssize_t written = write(h.fd, message.data(), message.size());
    if (written < 0 || static_cast<size_t>(written) != message.size()) {
        throw std::runtime_error("ipc.send() failed to write message: " + std::string(strerror(errno)));
    }
    return true;
#endif
}

// ipc.recv(handle) - blocking receive of a length-prefixed message
auto nativeIpcRecv(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("ipc.recv() takes exactly one handle (number) argument.");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.recv() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    IpcHandle h = ipcGetHandle(handle);
    if (!h.is_read) {
        throw std::runtime_error("ipc.recv() called on a write-only handle.");
    }
    uint8_t lenBuf[4];
    ssize_t n = read(h.fd, lenBuf, 4);
    if (n == 0) {
        throw std::runtime_error("ipc.recv() pipe closed (EOF).");
    }
    if (n != 4) {
        throw std::runtime_error("ipc.recv() failed to read message length: " + std::string(strerror(errno)));
    }
    uint32_t len = (static_cast<uint32_t>(lenBuf[0]) << 24) |
                   (static_cast<uint32_t>(lenBuf[1]) << 16) |
                   (static_cast<uint32_t>(lenBuf[2]) << 8)  |
                    static_cast<uint32_t>(lenBuf[3]);
    std::string message(len, '\0');
    ssize_t total = 0;
    while (static_cast<uint32_t>(total) < len) {
        ssize_t r = read(h.fd, &message[total], len - static_cast<uint32_t>(total));
        if (r <= 0) {
            throw std::runtime_error("ipc.recv() failed to read message body: " + std::string(strerror(errno)));
        }
        total += r;
    }
    return message;
#endif
}

// ipc.tryRecv(handle) - non-blocking receive; returns nil if no message is ready
auto nativeIpcTryRecv(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("ipc.tryRecv() takes exactly one handle (number) argument.");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.tryRecv() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    IpcHandle h = ipcGetHandle(handle);
    if (!h.is_read) {
        throw std::runtime_error("ipc.tryRecv() called on a write-only handle.");
    }
    // Check if data is available using select with zero timeout
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(h.fd, &readfds);
    struct timeval tv = {0, 0};
    int ready = select(h.fd + 1, &readfds, nullptr, nullptr, &tv);
    if (ready <= 0) {
        return Nil{};
    }
    uint8_t lenBuf[4];
    ssize_t n = read(h.fd, lenBuf, 4);
    if (n == 0) {
        return Nil{};
    }
    if (n != 4) {
        throw std::runtime_error("ipc.tryRecv() failed to read message length: " + std::string(strerror(errno)));
    }
    uint32_t len = (static_cast<uint32_t>(lenBuf[0]) << 24) |
                   (static_cast<uint32_t>(lenBuf[1]) << 16) |
                   (static_cast<uint32_t>(lenBuf[2]) << 8)  |
                    static_cast<uint32_t>(lenBuf[3]);
    std::string message(len, '\0');
    ssize_t total = 0;
    while (static_cast<uint32_t>(total) < len) {
        ssize_t r = read(h.fd, &message[total], len - static_cast<uint32_t>(total));
        if (r <= 0) {
            throw std::runtime_error("ipc.tryRecv() failed to read message body: " + std::string(strerror(errno)));
        }
        total += r;
    }
    return message;
#endif
}

// ipc.close(handle) - closes an IPC pipe handle
auto nativeIpcClose(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("ipc.close() takes exactly one handle (number) argument.");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.close() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    IpcHandle h = ipcGetHandle(handle);
    close(h.fd);
    ipcFreeHandle(handle);
    return Nil{};
#endif
}

// ipc.removePipe(name) - removes a named pipe from the filesystem
auto nativeIpcRemovePipe(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.removePipe() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.removePipe() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = ipcPipePath(name);
    if (unlink(path.c_str()) != 0 && errno != ENOENT) {
        throw std::runtime_error("ipc.removePipe() failed to remove pipe '" + name + "': " + strerror(errno));
    }
    return true;
#endif
}

// ============ std.net functions ============

namespace {

struct NetSocket {
    int fd;
    bool is_server;  // true = server (listening) socket
};

static std::mutex netMutex;
static std::unordered_map<int, NetSocket> netSocketMap;
static int nextNetHandle = 1;

static int netAllocHandle(int fd, bool is_server) {
    std::lock_guard<std::mutex> lock(netMutex);
    int id = nextNetHandle++;
    netSocketMap[id] = {fd, is_server};
    return id;
}

static NetSocket netGetHandle(int id) {
    std::lock_guard<std::mutex> lock(netMutex);
    auto it = netSocketMap.find(id);
    if (it == netSocketMap.end()) {
        throw std::runtime_error("net: invalid socket handle: " + std::to_string(id));
    }
    return it->second;
}

static void netFreeHandle(int id) {
    std::lock_guard<std::mutex> lock(netMutex);
    netSocketMap.erase(id);
}

}  // anonymous namespace

// net.connect(host, port) - connects to a TCP server; returns a socket handle
auto nativeNetConnect(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2 || !std::holds_alternative<std::string>(arguments[0]) ||
        !std::holds_alternative<double>(arguments[1])) {
        throw std::runtime_error("net.connect() takes a host (string) and port (number).");
    }
    const std::string& host = std::get<std::string>(arguments[0]);
    int port = static_cast<int>(std::get<double>(arguments[1]));
    if (port <= 0 || port > 65535) {
        throw std::runtime_error("net.connect(): port must be between 1 and 65535.");
    }

    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    std::string portStr = std::to_string(port);
    int rc = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (rc != 0) {
        throw std::runtime_error("net.connect(): failed to resolve host '" + host +
                                 "': " + std::string(gai_strerror(rc)));
    }

    int sock = -1;
    for (struct addrinfo* rp = res; rp != nullptr; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock < 0) continue;
        if (::connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;
        ::close(sock);
        sock = -1;
    }
    freeaddrinfo(res);
    if (sock < 0) {
        throw std::runtime_error("net.connect(): failed to connect to '" + host + ":" + portStr + "'");
    }

    int handle = netAllocHandle(sock, false);
    return static_cast<double>(handle);
}

// net.listen(port) - creates a TCP server socket bound to the given port; returns a server handle
auto nativeNetListen(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("net.listen() takes exactly one numeric argument (port).");
    }
    int port = static_cast<int>(std::get<double>(arguments[0]));
    if (port <= 0 || port > 65535) {
        throw std::runtime_error("net.listen(): port must be between 1 and 65535.");
    }

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
        // Fallback to IPv4 if IPv6 unavailable
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            throw std::runtime_error("net.listen(): failed to create socket: " + std::string(strerror(errno)));
        }
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        struct sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(static_cast<uint16_t>(port));
        if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(sock);
            throw std::runtime_error("net.listen(): failed to bind port " + std::to_string(port) +
                                     ": " + strerror(errno));
        }
    } else {
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        // Dual-stack: allow both IPv4 and IPv6
        int v6only = 0;
        setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &v6only, sizeof(v6only));
        struct sockaddr_in6 addr{};
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(static_cast<uint16_t>(port));
        if (bind(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(sock);
            throw std::runtime_error("net.listen(): failed to bind port " + std::to_string(port) +
                                     ": " + strerror(errno));
        }
    }

    if (listen(sock, 128) < 0) {
        ::close(sock);
        throw std::runtime_error("net.listen(): failed to listen on port " + std::to_string(port) +
                                 ": " + strerror(errno));
    }

    int handle = netAllocHandle(sock, true);
    return static_cast<double>(handle);
}

// net.accept(serverHandle [, timeoutMs]) - accepts a TCP connection; returns client handle or nil on timeout
auto nativeNetAccept(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.empty() || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("net.accept() takes a server handle (number) and an optional timeout (number).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    NetSocket srv = netGetHandle(handle);
    if (!srv.is_server) {
        throw std::runtime_error("net.accept(): handle is not a server socket.");
    }

    // Optional timeout in milliseconds
    int timeoutMs = -1;
    if (arguments.size() >= 2) {
        if (!std::holds_alternative<double>(arguments[1])) {
            throw std::runtime_error("net.accept(): timeout must be a number.");
        }
        timeoutMs = static_cast<int>(std::get<double>(arguments[1]));
    }

    if (timeoutMs >= 0) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(srv.fd, &readfds);
        struct timeval tv;
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        int ready = select(srv.fd + 1, &readfds, nullptr, nullptr, &tv);
        if (ready == 0) return Nil{};
        if (ready < 0) {
            throw std::runtime_error("net.accept(): select() failed: " + std::string(strerror(errno)));
        }
    }

    int client = ::accept(srv.fd, nullptr, nullptr);
    if (client < 0) {
        throw std::runtime_error("net.accept(): failed to accept connection: " + std::string(strerror(errno)));
    }
    int clientHandle = netAllocHandle(client, false);
    return static_cast<double>(clientHandle);
}

// net.send(handle, data) - sends a string over a TCP socket
auto nativeNetSend(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2 || !std::holds_alternative<double>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("net.send() takes a socket handle (number) and data (string).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    const std::string& data = std::get<std::string>(arguments[1]);
    NetSocket sock = netGetHandle(handle);
    if (sock.is_server) {
        throw std::runtime_error("net.send(): cannot send on a server (listening) socket.");
    }

    size_t total = 0;
    while (total < data.size()) {
#ifdef MSG_NOSIGNAL
        ssize_t n = ::send(sock.fd, data.data() + total, data.size() - total, MSG_NOSIGNAL);
#else
        ssize_t n = ::send(sock.fd, data.data() + total, data.size() - total, 0);
#endif
        if (n < 0) {
            if (errno == EPIPE || errno == ECONNRESET) {
                throw std::runtime_error("net.send(): connection closed by remote peer.");
            }
            throw std::runtime_error("net.send(): failed: " + std::string(strerror(errno)));
        }
        total += static_cast<size_t>(n);
    }
    return true;
}

// net.recv(handle [, bufsize]) - receives data from a TCP socket; returns empty string on connection close
auto nativeNetRecv(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.empty() || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("net.recv() takes a socket handle (number) and an optional buffer size (number).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    int bufsize = 4096;
    if (arguments.size() >= 2) {
        if (!std::holds_alternative<double>(arguments[1])) {
            throw std::runtime_error("net.recv(): buffer size must be a number.");
        }
        bufsize = static_cast<int>(std::get<double>(arguments[1]));
        if (bufsize <= 0) throw std::runtime_error("net.recv(): buffer size must be positive.");
    }
    NetSocket sock = netGetHandle(handle);
    if (sock.is_server) {
        throw std::runtime_error("net.recv(): cannot recv on a server (listening) socket.");
    }

    std::string buf(static_cast<size_t>(bufsize), '\0');
    ssize_t n = ::recv(sock.fd, &buf[0], static_cast<size_t>(bufsize), 0);
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return std::string{};
        }
        throw std::runtime_error("net.recv(): failed: " + std::string(strerror(errno)));
    }
    buf.resize(static_cast<size_t>(n));
    return buf;
}

// net.close(handle) - closes a TCP socket handle
auto nativeNetClose(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("net.close() takes exactly one socket handle (number).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    NetSocket sock = netGetHandle(handle);
    ::close(sock.fd);
    netFreeHandle(handle);
    return Nil{};
}

// net.setTimeout(handle, milliseconds) - sets send/recv timeout on a socket
auto nativeNetSetTimeout(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 2 || !std::holds_alternative<double>(arguments[0]) ||
        !std::holds_alternative<double>(arguments[1])) {
        throw std::runtime_error("net.setTimeout() takes a socket handle (number) and timeout in ms (number).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    int ms = static_cast<int>(std::get<double>(arguments[1]));
    if (ms < 0) throw std::runtime_error("net.setTimeout(): timeout must be non-negative.");
    NetSocket sock = netGetHandle(handle);

    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    setsockopt(sock.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock.fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    return Nil{};
}

// sleep(ms): pause execution for the given number of milliseconds
auto nativeSleep(Interpreter& /*interp*/, const std::vector<Value>& arguments) -> Value {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("sleep() takes exactly one numeric argument (milliseconds).");
    }
    double ms = std::get<double>(arguments[0]);
    if (ms < 0) {
        throw std::runtime_error("sleep() argument must be non-negative.");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(ms)));
    return Nil{};
}

void registerNativeFunctions(Interpreter& interp) {
    // Core functions
    interp.defineGlobal("print", Value{std::make_shared<NativeFunction>("print", -1, nativePrint)});
    interp.defineGlobal("clock", Value{std::make_shared<NativeFunction>("clock", 0, nativeClock)});
    interp.defineGlobal("len", Value{std::make_shared<NativeFunction>("len", 1, nativeLen)});
    interp.defineGlobal("str", Value{std::make_shared<NativeFunction>("str", 1, nativeStr)});

    // Concurrency functions
    interp.defineGlobal("spawn", Value{std::make_shared<NativeFunction>("spawn", 1, nativeSpawn)});
    interp.defineGlobal("await", Value{std::make_shared<NativeFunction>("await", 1, nativeAwait)});
    interp.defineGlobal("sleep", Value{std::make_shared<NativeFunction>("sleep", 1, nativeSleep)});

    // Array functions
    interp.defineGlobal("push", Value{std::make_shared<NativeFunction>("push", 2, nativePush)});
    interp.defineGlobal("pop", Value{std::make_shared<NativeFunction>("pop", 1, nativePop)});
    interp.defineGlobal("shift", Value{std::make_shared<NativeFunction>("shift", 1, nativeShift)});
    interp.defineGlobal("unshift", Value{std::make_shared<NativeFunction>("unshift", 2, nativeUnshift)});
    interp.defineGlobal("splice", Value{std::make_shared<NativeFunction>("splice", -1, nativeSplice)});

    // Map functions
    interp.defineGlobal("keys", Value{std::make_shared<NativeFunction>("keys", 1, nativeKeys)});
    interp.defineGlobal("values", Value{std::make_shared<NativeFunction>("values", 1, nativeValues)});
    interp.defineGlobal("hasKey", Value{std::make_shared<NativeFunction>("hasKey", 2, nativeHasKey)});
    interp.defineGlobal("has", Value{std::make_shared<NativeFunction>("has", 2, nativeHas)});
    interp.defineGlobal("delete", Value{std::make_shared<NativeFunction>("delete", 2, nativeDelete)});
    interp.defineGlobal("entries", Value{std::make_shared<NativeFunction>("entries", 1, nativeEntries)});

    // Set functions
    interp.defineGlobal("Set", Value{std::make_shared<NativeFunction>("Set", 0, nativeSet)});
    interp.defineGlobal("setAdd", Value{std::make_shared<NativeFunction>("setAdd", 2, nativeSetAdd)});
    interp.defineGlobal("setHas", Value{std::make_shared<NativeFunction>("setHas", 2, nativeSetHas)});
    interp.defineGlobal("setDelete", Value{std::make_shared<NativeFunction>("setDelete", 2, nativeSetDelete)});
    interp.defineGlobal("setSize", Value{std::make_shared<NativeFunction>("setSize", 1, nativeSetSize)});

    // std.math functions
    interp.defineGlobal("sqrt", Value{std::make_shared<NativeFunction>("sqrt", 1, nativeSqrt)});
    interp.defineGlobal("pow", Value{std::make_shared<NativeFunction>("pow", 2, nativePow)});
    interp.defineGlobal("abs", Value{std::make_shared<NativeFunction>("abs", 1, nativeAbs)});
    interp.defineGlobal("floor", Value{std::make_shared<NativeFunction>("floor", 1, nativeFloor)});
    interp.defineGlobal("ceil", Value{std::make_shared<NativeFunction>("ceil", 1, nativeCeil)});
    interp.defineGlobal("round", Value{std::make_shared<NativeFunction>("round", 1, nativeRound)});
    interp.defineGlobal("sin", Value{std::make_shared<NativeFunction>("sin", 1, nativeSin)});
    interp.defineGlobal("cos", Value{std::make_shared<NativeFunction>("cos", 1, nativeCos)});
    interp.defineGlobal("tan", Value{std::make_shared<NativeFunction>("tan", 1, nativeTan)});
    interp.defineGlobal("min", Value{std::make_shared<NativeFunction>("min", -1, nativeMin)});
    interp.defineGlobal("max", Value{std::make_shared<NativeFunction>("max", -1, nativeMax)});

    // std.string functions
    interp.defineGlobal("substring", Value{std::make_shared<NativeFunction>("substring", -1, nativeSubstring)});
    interp.defineGlobal("split", Value{std::make_shared<NativeFunction>("split", 2, nativeSplit)});
    interp.defineGlobal("join", Value{std::make_shared<NativeFunction>("join", 2, nativeJoin)});
    interp.defineGlobal("toUpper", Value{std::make_shared<NativeFunction>("toUpper", 1, nativeToUpper)});
    interp.defineGlobal("toLower", Value{std::make_shared<NativeFunction>("toLower", 1, nativeToLower)});
    interp.defineGlobal("trim", Value{std::make_shared<NativeFunction>("trim", 1, nativeTrim)});
    interp.defineGlobal("replace", Value{std::make_shared<NativeFunction>("replace", 3, nativeReplace)});
    interp.defineGlobal("startsWith", Value{std::make_shared<NativeFunction>("startsWith", 2, nativeStartsWith)});
    interp.defineGlobal("endsWith", Value{std::make_shared<NativeFunction>("endsWith", 2, nativeEndsWith)});
    interp.defineGlobal("indexOf", Value{std::make_shared<NativeFunction>("indexOf", 2, nativeIndexOf)});

    // std.array functions
    interp.defineGlobal("map", Value{std::make_shared<NativeFunction>("map", 2, nativeMap)});
    interp.defineGlobal("filter", Value{std::make_shared<NativeFunction>("filter", 2, nativeFilter)});
    interp.defineGlobal("reduce", Value{std::make_shared<NativeFunction>("reduce", -1, nativeReduce)});
    interp.defineGlobal("sort", Value{std::make_shared<NativeFunction>("sort", 1, nativeSort)});
    interp.defineGlobal("reverse", Value{std::make_shared<NativeFunction>("reverse", 1, nativeReverse)});
    interp.defineGlobal("concat", Value{std::make_shared<NativeFunction>("concat", 2, nativeConcat)});
    interp.defineGlobal("slice", Value{std::make_shared<NativeFunction>("slice", -1, nativeSlice)});

    // std.io functions
    interp.defineGlobal("readFile", Value{std::make_shared<NativeFunction>("readFile", 1, nativeReadFile)});
    interp.defineGlobal("writeFile", Value{std::make_shared<NativeFunction>("writeFile", 2, nativeWriteFile)});
    interp.defineGlobal("appendFile", Value{std::make_shared<NativeFunction>("appendFile", 2, nativeAppendFile)});
    interp.defineGlobal("fileExists", Value{std::make_shared<NativeFunction>("fileExists", 1, nativeFileExists)});
}

}  // namespace izi
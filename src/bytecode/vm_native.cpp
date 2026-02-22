#include "vm_native.hpp"
#include <chrono>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <random>
#include <sys/stat.h>
#include <thread>
#include <regex>
#include <ctime>
#ifndef _WIN32
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include <mutex>
#include <unordered_map>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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
    } else if (std::holds_alternative<std::shared_ptr<Set>>(arg)) {
        auto set = std::get<std::shared_ptr<Set>>(arg);
        return static_cast<double>(set->values.size());
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

Value vmNativeShift(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeUnshift(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSplice(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeHas(VM& vm, const std::vector<Value>& arguments) {
    // Alias for hasKey() for map compatibility
    return vmNativeHasKey(vm, arguments);
}

Value vmNativeDelete(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeEntries(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSetAdd(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSetHas(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSetDelete(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSetSize(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSet(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 0) {
        throw std::runtime_error("Set() takes no arguments.");
    }
    return std::make_shared<Set>();
}

// ============ std.math functions ============

Value vmNativeSqrt(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("sqrt() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    if (val < 0) {
        throw std::runtime_error("sqrt() argument must be non-negative.");
    }
    return std::sqrt(val);
}

Value vmNativePow(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("pow() takes exactly two arguments.");
    }
    double base = asNumber(arguments[0]);
    double exp = asNumber(arguments[1]);
    return std::pow(base, exp);
}

Value vmNativeAbs(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("abs() takes exactly one argument.");
    }
    return std::abs(asNumber(arguments[0]));
}

Value vmNativeFloor(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("floor() takes exactly one argument.");
    }
    return std::floor(asNumber(arguments[0]));
}

Value vmNativeCeil(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("ceil() takes exactly one argument.");
    }
    return std::ceil(asNumber(arguments[0]));
}

Value vmNativeRound(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("round() takes exactly one argument.");
    }
    return std::round(asNumber(arguments[0]));
}

Value vmNativeSin(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("sin() takes exactly one argument.");
    }
    return std::sin(asNumber(arguments[0]));
}

Value vmNativeCos(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("cos() takes exactly one argument.");
    }
    return std::cos(asNumber(arguments[0]));
}

Value vmNativeTan(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("tan() takes exactly one argument.");
    }
    return std::tan(asNumber(arguments[0]));
}

Value vmNativeMin(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.empty()) {
        throw std::runtime_error("min() requires at least one argument.");
    }
    double minVal = asNumber(arguments[0]);
    for (size_t i = 1; i < arguments.size(); ++i) {
        minVal = std::min(minVal, asNumber(arguments[i]));
    }
    return minVal;
}

Value vmNativeMax(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.empty()) {
        throw std::runtime_error("max() requires at least one argument.");
    }
    double maxVal = asNumber(arguments[0]);
    for (size_t i = 1; i < arguments.size(); ++i) {
        maxVal = std::max(maxVal, asNumber(arguments[i]));
    }
    return maxVal;
}

Value vmNativeTrunc(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("trunc() takes exactly one argument.");
    }
    return std::trunc(asNumber(arguments[0]));
}

Value vmNativeLog(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("log() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    if (val <= 0) {
        throw std::runtime_error("log() argument must be positive.");
    }
    return std::log(val);
}

Value vmNativeLog2(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("log2() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    if (val <= 0) {
        throw std::runtime_error("log2() argument must be positive.");
    }
    return std::log2(val);
}

Value vmNativeLog10(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("log10() takes exactly one argument.");
    }
    double val = asNumber(arguments[0]);
    if (val <= 0) {
        throw std::runtime_error("log10() argument must be positive.");
    }
    return std::log10(val);
}

Value vmNativeRandom(VM& vm, const std::vector<Value>& arguments) {
    if (!arguments.empty()) {
        throw std::runtime_error("random() takes no arguments.");
    }
    static thread_local std::mt19937_64 rng{std::random_device{}()};
    static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

Value vmNativeAsin(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("asin() takes exactly one argument.");
    }
    return std::asin(asNumber(arguments[0]));
}

Value vmNativeAcos(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("acos() takes exactly one argument.");
    }
    return std::acos(asNumber(arguments[0]));
}

Value vmNativeAtan(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("atan() takes exactly one argument.");
    }
    return std::atan(asNumber(arguments[0]));
}

Value vmNativeAtan2(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("atan2() takes exactly two arguments.");
    }
    return std::atan2(asNumber(arguments[0]), asNumber(arguments[1]));
}

Value vmNativeHypot(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.empty()) {
        throw std::runtime_error("hypot() requires at least one argument.");
    }
    double sumSq = 0.0;
    for (const auto& arg : arguments) {
        double v = asNumber(arg);
        sumSq += v * v;
    }
    return std::sqrt(sumSq);
}

// ============ std.string functions ============

Value vmNativeSubstring(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSplit(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeJoin(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeToUpper(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeToLower(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeTrim(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeReplace(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeStartsWith(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeEndsWith(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeIndexOf(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeContains(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("contains() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to contains() must be strings.");
    }
    const std::string& str = std::get<std::string>(arguments[0]);
    const std::string& substr = std::get<std::string>(arguments[1]);
    return str.find(substr) != std::string::npos;
}

// ============ std.array functions ============

Value vmNativeMap(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("map() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("First argument to map() must be an array.");
    }
    if (!std::holds_alternative<std::shared_ptr<VmCallable>>(arguments[1])) {
        throw std::runtime_error("Second argument to map() must be a function.");
    }

    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto func = std::get<std::shared_ptr<VmCallable>>(arguments[1]);
    auto result = std::make_shared<Array>();

    for (const auto& elem : arr->elements) {
        result->elements.push_back(func->call(vm, {elem}));
    }
    return result;
}

Value vmNativeFilter(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("filter() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("First argument to filter() must be an array.");
    }
    if (!std::holds_alternative<std::shared_ptr<VmCallable>>(arguments[1])) {
        throw std::runtime_error("Second argument to filter() must be a function.");
    }

    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto func = std::get<std::shared_ptr<VmCallable>>(arguments[1]);
    auto result = std::make_shared<Array>();

    for (const auto& elem : arr->elements) {
        Value testResult = func->call(vm, {elem});
        if (isTruthy(testResult)) {
            result->elements.push_back(elem);
        }
    }
    return result;
}

Value vmNativeReduce(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() < 2 || arguments.size() > 3) {
        throw std::runtime_error("reduce() takes 2 or 3 arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("First argument to reduce() must be an array.");
    }
    if (!std::holds_alternative<std::shared_ptr<VmCallable>>(arguments[1])) {
        throw std::runtime_error("Second argument to reduce() must be a function.");
    }

    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto func = std::get<std::shared_ptr<VmCallable>>(arguments[1]);

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
        accumulator = func->call(vm, {accumulator, arr->elements[i]});
    }
    return accumulator;
}

Value vmNativeSort(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() < 1 || arguments.size() > 2) {
        throw std::runtime_error("sort() takes 1 or 2 arguments.");
    }
    if (!std::holds_alternative<std::shared_ptr<Array>>(arguments[0])) {
        throw std::runtime_error("Argument to sort() must be an array.");
    }

    auto arr = std::get<std::shared_ptr<Array>>(arguments[0]);
    auto result = std::make_shared<Array>(*arr);

    if (arguments.size() == 2) {
        if (!std::holds_alternative<std::shared_ptr<VmCallable>>(arguments[1])) {
            throw std::runtime_error("Second argument to sort() must be a function.");
        }
        auto comparator = std::get<std::shared_ptr<VmCallable>>(arguments[1]);
        std::sort(result->elements.begin(), result->elements.end(), [&](const Value& a, const Value& b) {
            Value cmpResult = comparator->call(vm, {a, b});
            return asNumber(cmpResult) < 0;
        });
    } else {
        std::sort(result->elements.begin(), result->elements.end(), [](const Value& a, const Value& b) {
            if (std::holds_alternative<double>(a) && std::holds_alternative<double>(b)) {
                return std::get<double>(a) < std::get<double>(b);
            }
            if (std::holds_alternative<std::string>(a) && std::holds_alternative<std::string>(b)) {
                return std::get<std::string>(a) < std::get<std::string>(b);
            }
            return false;
        });
    }
    return result;
}

Value vmNativeReverse(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeConcat(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeSlice(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeReadFile(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeWriteFile(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeAppendFile(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeFileExists(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeLogInfo(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.info() takes exactly one argument.");
    }
    std::cout << "[INFO] " << valueToString(arguments[0]) << "\n";
    return Nil{};
}

Value vmNativeLogWarn(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.warn() takes exactly one argument.");
    }
    std::cout << "[WARN] " << valueToString(arguments[0]) << "\n";
    return Nil{};
}

Value vmNativeLogError(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.error() takes exactly one argument.");
    }
    std::cerr << "[ERROR] " << valueToString(arguments[0]) << "\n";
    return Nil{};
}

Value vmNativeLogDebug(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("log.debug() takes exactly one argument.");
    }
    std::cout << "[DEBUG] " << valueToString(arguments[0]) << "\n";
    return Nil{};
}

// std.time functions
Value vmNativeTimeNow(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 0) {
        throw std::runtime_error("time.now() takes no arguments.");
    }

    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return static_cast<double>(ms) / 1000.0;
}

Value vmNativeTimeSleep(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeTimeFormat(VM& vm, const std::vector<Value>& arguments) {
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

// std.json helper functions (shared with interpreter version)
static Value vmJsonValueFromString(const std::string& jsonStr, size_t& pos);

static void vmSkipWhitespace(const std::string& str, size_t& pos) {
    while (pos < str.size() && std::isspace(str[pos])) {
        ++pos;
    }
}

static Value vmParseJsonNull(const std::string& str, size_t& pos) {
    if (str.substr(pos, 4) == "null") {
        pos += 4;
        return Nil{};
    }
    throw std::runtime_error("Invalid JSON: expected 'null'");
}

static Value vmParseJsonBool(const std::string& str, size_t& pos) {
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

static Value vmParseJsonNumber(const std::string& str, size_t& pos) {
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

static Value vmParseJsonString(const std::string& str, size_t& pos) {
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
    ++pos;

    return result;
}

static Value vmParseJsonArray(const std::string& str, size_t& pos) {
    if (str[pos] != '[') {
        throw std::runtime_error("Invalid JSON: expected array");
    }
    ++pos;

    auto arr = std::make_shared<Array>();
    vmSkipWhitespace(str, pos);

    if (pos < str.size() && str[pos] == ']') {
        ++pos;
        return arr;
    }

    while (pos < str.size()) {
        arr->elements.push_back(vmJsonValueFromString(str, pos));
        vmSkipWhitespace(str, pos);

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
        vmSkipWhitespace(str, pos);
    }

    throw std::runtime_error("Invalid JSON: unterminated array");
}

static Value vmParseJsonObject(const std::string& str, size_t& pos) {
    if (str[pos] != '{') {
        throw std::runtime_error("Invalid JSON: expected object");
    }
    ++pos;

    auto map = std::make_shared<Map>();
    vmSkipWhitespace(str, pos);

    if (pos < str.size() && str[pos] == '}') {
        ++pos;
        return map;
    }

    while (pos < str.size()) {
        vmSkipWhitespace(str, pos);

        if (str[pos] != '"') {
            throw std::runtime_error("Invalid JSON: expected string key in object");
        }

        Value keyVal = vmParseJsonString(str, pos);
        std::string key = std::get<std::string>(keyVal);

        vmSkipWhitespace(str, pos);
        if (pos >= str.size() || str[pos] != ':') {
            throw std::runtime_error("Invalid JSON: expected ':' after object key");
        }
        ++pos;
        vmSkipWhitespace(str, pos);

        Value value = vmJsonValueFromString(str, pos);
        map->entries[key] = value;

        vmSkipWhitespace(str, pos);

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

static Value vmJsonValueFromString(const std::string& jsonStr, size_t& pos) {
    vmSkipWhitespace(jsonStr, pos);

    if (pos >= jsonStr.size()) {
        throw std::runtime_error("Invalid JSON: unexpected end of input");
    }

    char c = jsonStr[pos];

    if (c == 'n') return vmParseJsonNull(jsonStr, pos);
    if (c == 't' || c == 'f') return vmParseJsonBool(jsonStr, pos);
    if (c == '"') return vmParseJsonString(jsonStr, pos);
    if (c == '[') return vmParseJsonArray(jsonStr, pos);
    if (c == '{') return vmParseJsonObject(jsonStr, pos);
    if (c == '-' || std::isdigit(c)) return vmParseJsonNumber(jsonStr, pos);

    throw std::runtime_error("Invalid JSON: unexpected character");
}

static std::string vmValueToJson(const Value& value) {
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
            oss << vmValueToJson(arr->elements[i]);
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
            oss << '"' << key << "\":" << vmValueToJson(val);
        }
        oss << '}';
        return oss.str();
    }

    return "null";
}

Value vmNativeJsonParse(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("json.parse() takes exactly one argument.");
    }
    if (!std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("Argument to json.parse() must be a string.");
    }

    std::string jsonStr = std::get<std::string>(arguments[0]);
    size_t pos = 0;

    try {
        Value result = vmJsonValueFromString(jsonStr, pos);
        vmSkipWhitespace(jsonStr, pos);
        if (pos < jsonStr.size()) {
            throw std::runtime_error("Invalid JSON: unexpected characters after value");
        }
        return result;
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("JSON parse error: ") + e.what());
    }
}

Value vmNativeJsonStringify(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 1) {
        throw std::runtime_error("json.stringify() takes exactly one argument.");
    }

    try {
        return vmValueToJson(arguments[0]);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("JSON stringify error: ") + e.what());
    }
}

// std.regex functions
Value vmNativeRegexMatch(VM& vm, const std::vector<Value>& arguments) {
    if (arguments.size() != 2) {
        throw std::runtime_error("regex.match() takes exactly two arguments.");
    }
    if (!std::holds_alternative<std::string>(arguments[0]) || !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("Both arguments to regex.match() must be strings.");
    }

    std::string text = std::get<std::string>(arguments[0]);
    std::string pattern = std::get<std::string>(arguments[1]);

    try {
        std::regex re(pattern);
        auto result = std::make_shared<Array>();
        auto searchStart = text.cbegin();
        std::smatch match;
        while (std::regex_search(searchStart, text.cend(), match, re)) {
            result->elements.push_back(match[0].str());
            auto next = match.suffix().first;
            if (next == searchStart) {
                if (searchStart == text.cend()) break;
                ++searchStart;
            } else {
                searchStart = next;
            }
        }
        return result;
    } catch (const std::regex_error& e) {
        throw std::runtime_error(std::string("Regex error: ") + e.what());
    }
}

Value vmNativeRegexReplace(VM& vm, const std::vector<Value>& arguments) {
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

Value vmNativeRegexTest(VM& vm, const std::vector<Value>& arguments) {
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

// ============ std.ipc VM functions ============

namespace {

struct VmIpcHandle {
    int fd;
    bool is_read;
    std::string path;
};

static std::mutex vmIpcMutex;
static std::unordered_map<int, VmIpcHandle> vmIpcHandleMap;
static int vmNextIpcHandle = 1;

static std::string vmIpcPipePath(const std::string& name) {
    return "/tmp/izi_ipc_" + name;
}

static int vmIpcAllocHandle(int fd, bool is_read, const std::string& path) {
    std::lock_guard<std::mutex> lock(vmIpcMutex);
    int id = vmNextIpcHandle++;
    vmIpcHandleMap[id] = {fd, is_read, path};
    return id;
}

static VmIpcHandle vmIpcGetHandle(int id) {
    std::lock_guard<std::mutex> lock(vmIpcMutex);
    auto it = vmIpcHandleMap.find(id);
    if (it == vmIpcHandleMap.end()) {
        throw std::runtime_error("Invalid IPC handle: " + std::to_string(id));
    }
    return it->second;
}

static void vmIpcFreeHandle(int id) {
    std::lock_guard<std::mutex> lock(vmIpcMutex);
    vmIpcHandleMap.erase(id);
}

}  // anonymous namespace

Value vmNativeIpcCreatePipe(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.createPipe() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.createPipe() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = vmIpcPipePath(name);
    if (mkfifo(path.c_str(), 0600) != 0 && errno != EEXIST) {
        throw std::runtime_error("ipc.createPipe() failed to create pipe '" + name + "': " + strerror(errno));
    }
    return true;
#endif
}

Value vmNativeIpcOpenRead(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.openRead() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.openRead() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = vmIpcPipePath(name);
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error("ipc.openRead() failed to open pipe '" + name + "': " + strerror(errno));
    }
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    int handle = vmIpcAllocHandle(fd, true, path);
    return static_cast<double>(handle);
#endif
}

Value vmNativeIpcOpenWrite(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.openWrite() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.openWrite() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = vmIpcPipePath(name);
    int fd = open(path.c_str(), O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error("ipc.openWrite() failed to open pipe '" + name +
                                 "': " + strerror(errno) +
                                 " (ensure a reader has opened the pipe first)");
    }
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    int handle = vmIpcAllocHandle(fd, false, path);
    return static_cast<double>(handle);
#endif
}

Value vmNativeIpcSend(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 2 || !std::holds_alternative<double>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("ipc.send() takes a handle (number) and a message (string).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.send() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    const std::string& message = std::get<std::string>(arguments[1]);
    VmIpcHandle h = vmIpcGetHandle(handle);
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

Value vmNativeIpcRecv(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("ipc.recv() takes exactly one handle (number) argument.");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.recv() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    VmIpcHandle h = vmIpcGetHandle(handle);
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

Value vmNativeIpcTryRecv(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("ipc.tryRecv() takes exactly one handle (number) argument.");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.tryRecv() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    VmIpcHandle h = vmIpcGetHandle(handle);
    if (!h.is_read) {
        throw std::runtime_error("ipc.tryRecv() called on a write-only handle.");
    }
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

Value vmNativeIpcClose(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("ipc.close() takes exactly one handle (number) argument.");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.close() is not supported on Windows.");
#else
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    VmIpcHandle h = vmIpcGetHandle(handle);
    close(h.fd);
    vmIpcFreeHandle(handle);
    return Nil{};
#endif
}

Value vmNativeIpcRemovePipe(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<std::string>(arguments[0])) {
        throw std::runtime_error("ipc.removePipe() takes exactly one string argument (pipe name).");
    }
#ifdef _WIN32
    throw std::runtime_error("ipc.removePipe() is not supported on Windows.");
#else
    const std::string& name = std::get<std::string>(arguments[0]);
    std::string path = vmIpcPipePath(name);
    if (unlink(path.c_str()) != 0 && errno != ENOENT) {
        throw std::runtime_error("ipc.removePipe() failed to remove pipe '" + name + "': " + strerror(errno));
    }
    return true;
#endif
}

// ============ std.net functions ============

namespace {

struct VmNetSocket {
    int fd;
    bool is_server;
};

static std::mutex vmNetMutex;
static std::unordered_map<int, VmNetSocket> vmNetSocketMap;
static int vmNextNetHandle = 1;

static int vmNetAllocHandle(int fd, bool is_server) {
    std::lock_guard<std::mutex> lock(vmNetMutex);
    int id = vmNextNetHandle++;
    vmNetSocketMap[id] = {fd, is_server};
    return id;
}

static VmNetSocket vmNetGetHandle(int id) {
    std::lock_guard<std::mutex> lock(vmNetMutex);
    auto it = vmNetSocketMap.find(id);
    if (it == vmNetSocketMap.end()) {
        throw std::runtime_error("net: invalid socket handle: " + std::to_string(id));
    }
    return it->second;
}

static void vmNetFreeHandle(int id) {
    std::lock_guard<std::mutex> lock(vmNetMutex);
    vmNetSocketMap.erase(id);
}

}  // anonymous namespace

Value vmNativeNetConnect(VM& /*vm*/, const std::vector<Value>& arguments) {
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

    int handle = vmNetAllocHandle(sock, false);
    return static_cast<double>(handle);
}

Value vmNativeNetListen(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("net.listen() takes exactly one numeric argument (port).");
    }
    int port = static_cast<int>(std::get<double>(arguments[0]));
    if (port <= 0 || port > 65535) {
        throw std::runtime_error("net.listen(): port must be between 1 and 65535.");
    }

    int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock < 0) {
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

    int handle = vmNetAllocHandle(sock, true);
    return static_cast<double>(handle);
}

Value vmNativeNetAccept(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.empty() || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("net.accept() takes a server handle (number) and an optional timeout (number).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    VmNetSocket srv = vmNetGetHandle(handle);
    if (!srv.is_server) {
        throw std::runtime_error("net.accept(): handle is not a server socket.");
    }

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
    int clientHandle = vmNetAllocHandle(client, false);
    return static_cast<double>(clientHandle);
}

Value vmNativeNetSend(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 2 || !std::holds_alternative<double>(arguments[0]) ||
        !std::holds_alternative<std::string>(arguments[1])) {
        throw std::runtime_error("net.send() takes a socket handle (number) and data (string).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    const std::string& data = std::get<std::string>(arguments[1]);
    VmNetSocket sock = vmNetGetHandle(handle);
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

Value vmNativeNetRecv(VM& /*vm*/, const std::vector<Value>& arguments) {
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
    VmNetSocket sock = vmNetGetHandle(handle);
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

Value vmNativeNetClose(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 1 || !std::holds_alternative<double>(arguments[0])) {
        throw std::runtime_error("net.close() takes exactly one socket handle (number).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    VmNetSocket sock = vmNetGetHandle(handle);
    ::close(sock.fd);
    vmNetFreeHandle(handle);
    return Nil{};
}

Value vmNativeNetSetTimeout(VM& /*vm*/, const std::vector<Value>& arguments) {
    if (arguments.size() != 2 || !std::holds_alternative<double>(arguments[0]) ||
        !std::holds_alternative<double>(arguments[1])) {
        throw std::runtime_error("net.setTimeout() takes a socket handle (number) and timeout in ms (number).");
    }
    int handle = static_cast<int>(std::get<double>(arguments[0]));
    int ms = static_cast<int>(std::get<double>(arguments[1]));
    if (ms < 0) throw std::runtime_error("net.setTimeout(): timeout must be non-negative.");
    VmNetSocket sock = vmNetGetHandle(handle);

    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    setsockopt(sock.fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock.fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    return Nil{};
}

void registerVmNatives(VM& vm) {
    // Core functions
    vm.setGlobal("print", std::make_shared<VmNativeFunction>("print", -1, vmNativePrint));
    vm.setGlobal("len", std::make_shared<VmNativeFunction>("len", 1, vmNativeLen));
    vm.setGlobal("clock", std::make_shared<VmNativeFunction>("clock", 0, vmNativeClock));

    // Array functions
    vm.setGlobal("push", std::make_shared<VmNativeFunction>("push", 2, vmNativePush));
    vm.setGlobal("pop", std::make_shared<VmNativeFunction>("pop", 1, vmNativePop));
    vm.setGlobal("shift", std::make_shared<VmNativeFunction>("shift", 1, vmNativeShift));
    vm.setGlobal("unshift", std::make_shared<VmNativeFunction>("unshift", 2, vmNativeUnshift));
    vm.setGlobal("splice", std::make_shared<VmNativeFunction>("splice", -1, vmNativeSplice));

    // Map functions
    vm.setGlobal("keys", std::make_shared<VmNativeFunction>("keys", 1, vmNativeKeys));
    vm.setGlobal("values", std::make_shared<VmNativeFunction>("values", 1, vmNativeValues));
    vm.setGlobal("hasKey", std::make_shared<VmNativeFunction>("hasKey", 2, vmNativeHasKey));
    vm.setGlobal("has", std::make_shared<VmNativeFunction>("has", 2, vmNativeHas));
    vm.setGlobal("delete", std::make_shared<VmNativeFunction>("delete", 2, vmNativeDelete));
    vm.setGlobal("entries", std::make_shared<VmNativeFunction>("entries", 1, vmNativeEntries));

    // Set functions
    vm.setGlobal("Set", std::make_shared<VmNativeFunction>("Set", 0, vmNativeSet));
    vm.setGlobal("setAdd", std::make_shared<VmNativeFunction>("setAdd", 2, vmNativeSetAdd));
    vm.setGlobal("setHas", std::make_shared<VmNativeFunction>("setHas", 2, vmNativeSetHas));
    vm.setGlobal("setDelete", std::make_shared<VmNativeFunction>("setDelete", 2, vmNativeSetDelete));
    vm.setGlobal("setSize", std::make_shared<VmNativeFunction>("setSize", 1, vmNativeSetSize));

    // std.math functions
    vm.setGlobal("sqrt", std::make_shared<VmNativeFunction>("sqrt", 1, vmNativeSqrt));
    vm.setGlobal("pow", std::make_shared<VmNativeFunction>("pow", 2, vmNativePow));
    vm.setGlobal("abs", std::make_shared<VmNativeFunction>("abs", 1, vmNativeAbs));
    vm.setGlobal("floor", std::make_shared<VmNativeFunction>("floor", 1, vmNativeFloor));
    vm.setGlobal("ceil", std::make_shared<VmNativeFunction>("ceil", 1, vmNativeCeil));
    vm.setGlobal("round", std::make_shared<VmNativeFunction>("round", 1, vmNativeRound));
    vm.setGlobal("trunc", std::make_shared<VmNativeFunction>("trunc", 1, vmNativeTrunc));
    vm.setGlobal("log", std::make_shared<VmNativeFunction>("log", 1, vmNativeLog));
    vm.setGlobal("log2", std::make_shared<VmNativeFunction>("log2", 1, vmNativeLog2));
    vm.setGlobal("log10", std::make_shared<VmNativeFunction>("log10", 1, vmNativeLog10));
    vm.setGlobal("sin", std::make_shared<VmNativeFunction>("sin", 1, vmNativeSin));
    vm.setGlobal("cos", std::make_shared<VmNativeFunction>("cos", 1, vmNativeCos));
    vm.setGlobal("tan", std::make_shared<VmNativeFunction>("tan", 1, vmNativeTan));
    vm.setGlobal("asin", std::make_shared<VmNativeFunction>("asin", 1, vmNativeAsin));
    vm.setGlobal("acos", std::make_shared<VmNativeFunction>("acos", 1, vmNativeAcos));
    vm.setGlobal("atan", std::make_shared<VmNativeFunction>("atan", 1, vmNativeAtan));
    vm.setGlobal("atan2", std::make_shared<VmNativeFunction>("atan2", 2, vmNativeAtan2));
    vm.setGlobal("hypot", std::make_shared<VmNativeFunction>("hypot", -1, vmNativeHypot));
    vm.setGlobal("min", std::make_shared<VmNativeFunction>("min", -1, vmNativeMin));
    vm.setGlobal("max", std::make_shared<VmNativeFunction>("max", -1, vmNativeMax));

    // std.string functions
    vm.setGlobal("substring", std::make_shared<VmNativeFunction>("substring", -1, vmNativeSubstring));
    vm.setGlobal("split", std::make_shared<VmNativeFunction>("split", 2, vmNativeSplit));
    vm.setGlobal("join", std::make_shared<VmNativeFunction>("join", 2, vmNativeJoin));
    vm.setGlobal("toUpper", std::make_shared<VmNativeFunction>("toUpper", 1, vmNativeToUpper));
    vm.setGlobal("toLower", std::make_shared<VmNativeFunction>("toLower", 1, vmNativeToLower));
    vm.setGlobal("trim", std::make_shared<VmNativeFunction>("trim", 1, vmNativeTrim));
    vm.setGlobal("replace", std::make_shared<VmNativeFunction>("replace", 3, vmNativeReplace));
    vm.setGlobal("startsWith", std::make_shared<VmNativeFunction>("startsWith", 2, vmNativeStartsWith));
    vm.setGlobal("endsWith", std::make_shared<VmNativeFunction>("endsWith", 2, vmNativeEndsWith));
    vm.setGlobal("indexOf", std::make_shared<VmNativeFunction>("indexOf", 2, vmNativeIndexOf));
    vm.setGlobal("contains", std::make_shared<VmNativeFunction>("contains", 2, vmNativeContains));

    // std.array functions
    vm.setGlobal("map", std::make_shared<VmNativeFunction>("map", 2, vmNativeMap));
    vm.setGlobal("filter", std::make_shared<VmNativeFunction>("filter", 2, vmNativeFilter));
    vm.setGlobal("reduce", std::make_shared<VmNativeFunction>("reduce", -1, vmNativeReduce));
    vm.setGlobal("sort", std::make_shared<VmNativeFunction>("sort", -1, vmNativeSort));
    vm.setGlobal("reverse", std::make_shared<VmNativeFunction>("reverse", 1, vmNativeReverse));
    vm.setGlobal("concat", std::make_shared<VmNativeFunction>("concat", 2, vmNativeConcat));
    vm.setGlobal("slice", std::make_shared<VmNativeFunction>("slice", -1, vmNativeSlice));

    // std.io functions
    vm.setGlobal("readFile", std::make_shared<VmNativeFunction>("readFile", 1, vmNativeReadFile));
    vm.setGlobal("writeFile", std::make_shared<VmNativeFunction>("writeFile", 2, vmNativeWriteFile));
    vm.setGlobal("appendFile", std::make_shared<VmNativeFunction>("appendFile", 2, vmNativeAppendFile));
    vm.setGlobal("fileExists", std::make_shared<VmNativeFunction>("fileExists", 1, vmNativeFileExists));
}

}  // namespace izi

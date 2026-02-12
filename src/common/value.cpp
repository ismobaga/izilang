#include "value.hpp"
#include "bytecode/mv_callable.hpp"
#include "interp/izi_class.hpp"
#include "bytecode/vm_class.hpp"
#include <sstream>
#include <cmath>

namespace izi {

std::string getInstanceClassName(const Instance& instance) {
    if (std::holds_alternative<std::shared_ptr<IziClass>>(instance.klass)) {
        return std::get<std::shared_ptr<IziClass>>(instance.klass)->name();
    } else if (std::holds_alternative<std::shared_ptr<VmClass>>(instance.klass)) {
        return std::get<std::shared_ptr<VmClass>>(instance.klass)->name();
    }
    return "unknown";
}

std::string valueToString(const Value& v) {
    std::ostringstream oss;
    if (std::holds_alternative<Nil>(v)) {
        oss << "nil";
    } else if (std::holds_alternative<double>(v)) {
        double num = std::get<double>(v);
        // Check if it's a whole number to avoid trailing .0
        if (num == std::floor(num) && std::isfinite(num)) {
            oss << static_cast<long long>(num);
        } else {
            oss << num;
        }
    } else if (std::holds_alternative<bool>(v)) {
        oss << (std::get<bool>(v) ? "true" : "false");
    } else if (std::holds_alternative<std::string>(v)) {
        oss << std::get<std::string>(v);
    } else if (std::holds_alternative<std::shared_ptr<Callable>>(v)) {
        oss << "<fn " << std::get<std::shared_ptr<Callable>>(v)->name() << ">";
    } else if (std::holds_alternative<std::shared_ptr<Array>>(v)) {
        oss << "[...]";  // Simplified for string interpolation
    } else if (std::holds_alternative<std::shared_ptr<Map>>(v)) {
        oss << "{...}";  // Simplified for string interpolation
    } else if (std::holds_alternative<std::shared_ptr<Set>>(v)) {
        oss << "Set(...)";  // Simplified for string interpolation
    } else if (std::holds_alternative<std::shared_ptr<VmCallable>>(v)) {
        oss << "<vm fn " << std::get<std::shared_ptr<VmCallable>>(v)->name() << ">";
    } else if (std::holds_alternative<std::shared_ptr<Instance>>(v)) {
        auto instance = std::get<std::shared_ptr<Instance>>(v);
        oss << "<" << getInstanceClassName(*instance) << " instance>";
    } else {
        oss << "<unknown>";
    }
    return oss.str();
}

void printValue(const Value& v) {
    if (std::holds_alternative<Nil>(v)) {
        std::cout << "nil";
    } else if (std::holds_alternative<double>(v)) {
        std::cout << std::get<double>(v);
    } else if (std::holds_alternative<bool>(v)) {
        std::cout << (std::get<bool>(v) ? "true" : "false");
    } else if (std::holds_alternative<std::string>(v)) {
        std::cout << std::get<std::string>(v);
    } else if (std::holds_alternative<std::shared_ptr<Callable>>(v)) {
        std::cout << "<fn " << std::get<std::shared_ptr<Callable>>(v)->name() << ">";
    } else if (std::holds_alternative<std::shared_ptr<Array>>(v)) {
        printArray(*std::get<std::shared_ptr<Array>>(v));
    } else if (std::holds_alternative<std::shared_ptr<Map>>(v)) {
        printMap(*std::get<std::shared_ptr<Map>>(v));
    } else if (std::holds_alternative<std::shared_ptr<Set>>(v)) {
        printSet(*std::get<std::shared_ptr<Set>>(v));
    } else if (std::holds_alternative<std::shared_ptr<VmCallable>>(v)) {
        std::cout << "<vm fn " << std::get<std::shared_ptr<VmCallable>>(v)->name() << ">";
    } else if (std::holds_alternative<std::shared_ptr<Instance>>(v)) {
        auto instance = std::get<std::shared_ptr<Instance>>(v);
        std::cout << "<" << getInstanceClassName(*instance) << " instance>";
    } else {
        std::cout << "<unknown>";
    }
}

} // namespace izi

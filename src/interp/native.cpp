#include "native.hpp"
#include "interpreter.hpp"
#include <chrono>

namespace izi {
auto nativePrint(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    for (const auto& arg : arguments) {
        printValue(arg);
        std::cout << " ";
    }
    std::cout << std::endl;
    return Nil{};
}

auto nativeClock(Interpreter& interp, const std::vector<Value>& arguments) -> Value {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    return static_cast<double>(ms) / 1000.0;
}

void registerNativeFunctions(Interpreter& interpreter) {
    interpreter.defineGlobal("print", Value{std::make_shared<NativeFunction>(
        "print", -1, nativePrint)});
    interpreter.defineGlobal("clock", Value{std::make_shared<NativeFunction>(
        "clock", 0, nativeClock)});

    }

}
#pragma once

#include <functional>
#include <string>
#include <vector>

#include "../common/value.hpp"
#include "interpreter.hpp"

namespace izi {

class NativeFunction : public Callable {
   public:
    using NativeFn = std::function<Value(Interpreter&, const std::vector<Value>&)>;

    NativeFunction(std::string name, int arity, NativeFn fn)
        : name_(std::move(name)), arity_(arity), fn_(std::move(fn)) {}

    std::string name() const { return name_; }
    int arity() const override { return arity_; }

    Value call(Interpreter& interp, const std::vector<Value>& arguments) override {
        return fn_(interp, arguments);
    }

   private:
    std::string name_;
    int arity_;
    NativeFn fn_;
};

auto nativePrint(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeClock(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeLen(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativePush(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativePop(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeKeys(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeValues(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeHasKey(Interpreter& interp, const std::vector<Value>& arguments) -> Value;


void registerNativeFunctions(Interpreter& interpreter);

}  // namespace izi
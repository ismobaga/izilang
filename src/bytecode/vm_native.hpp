#pragma once

#include "mv_callable.hpp"
#include "vm.hpp"
#include <functional>

namespace izi {

class VmNativeFunction : public VmCallable {
public:
    using NativeFn = std::function<Value(VM&, const std::vector<Value>&)>;

    VmNativeFunction(std::string name, int arity, NativeFn fn)
        : name_(std::move(name)), arity_(arity), fn_(std::move(fn)) {}

    std::string name() const override { return name_; }
    int arity() const override { return arity_; }

    Value call(VM& vm, const std::vector<Value>& arguments) override {
        return fn_(vm, arguments);
    }

private:
    std::string name_;
    int arity_;
    NativeFn fn_;
};

// Native function implementations
Value vmNativePrint(VM& vm, const std::vector<Value>& arguments);
Value vmNativeLen(VM& vm, const std::vector<Value>& arguments);
Value vmNativeClock(VM& vm, const std::vector<Value>& arguments);
Value vmNativePush(VM& vm, const std::vector<Value>& arguments);
Value vmNativePop(VM& vm, const std::vector<Value>& arguments);
Value vmNativeKeys(VM& vm, const std::vector<Value>& arguments);
Value vmNativeValues(VM& vm, const std::vector<Value>& arguments);
Value vmNativeHasKey(VM& vm, const std::vector<Value>& arguments);

void registerVmNatives(VM& vm);

} // namespace izi

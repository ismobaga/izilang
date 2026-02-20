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

    Value call(VM& vm, const std::vector<Value>& arguments) override { return fn_(vm, arguments); }

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
Value vmNativeShift(VM& vm, const std::vector<Value>& arguments);
Value vmNativeUnshift(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSplice(VM& vm, const std::vector<Value>& arguments);
Value vmNativeKeys(VM& vm, const std::vector<Value>& arguments);
Value vmNativeValues(VM& vm, const std::vector<Value>& arguments);
Value vmNativeHasKey(VM& vm, const std::vector<Value>& arguments);
Value vmNativeHas(VM& vm, const std::vector<Value>& arguments);
Value vmNativeDelete(VM& vm, const std::vector<Value>& arguments);
Value vmNativeEntries(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSetAdd(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSetHas(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSetDelete(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSetSize(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSet(VM& vm, const std::vector<Value>& arguments);

// std.math native functions
Value vmNativeSqrt(VM& vm, const std::vector<Value>& arguments);
Value vmNativePow(VM& vm, const std::vector<Value>& arguments);
Value vmNativeAbs(VM& vm, const std::vector<Value>& arguments);
Value vmNativeFloor(VM& vm, const std::vector<Value>& arguments);
Value vmNativeCeil(VM& vm, const std::vector<Value>& arguments);
Value vmNativeRound(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSin(VM& vm, const std::vector<Value>& arguments);
Value vmNativeCos(VM& vm, const std::vector<Value>& arguments);
Value vmNativeTan(VM& vm, const std::vector<Value>& arguments);
Value vmNativeMin(VM& vm, const std::vector<Value>& arguments);
Value vmNativeMax(VM& vm, const std::vector<Value>& arguments);

// std.string native functions
Value vmNativeSubstring(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSplit(VM& vm, const std::vector<Value>& arguments);
Value vmNativeJoin(VM& vm, const std::vector<Value>& arguments);
Value vmNativeToUpper(VM& vm, const std::vector<Value>& arguments);
Value vmNativeToLower(VM& vm, const std::vector<Value>& arguments);
Value vmNativeTrim(VM& vm, const std::vector<Value>& arguments);
Value vmNativeReplace(VM& vm, const std::vector<Value>& arguments);
Value vmNativeStartsWith(VM& vm, const std::vector<Value>& arguments);
Value vmNativeEndsWith(VM& vm, const std::vector<Value>& arguments);
Value vmNativeIndexOf(VM& vm, const std::vector<Value>& arguments);

// std.array native functions
Value vmNativeMap(VM& vm, const std::vector<Value>& arguments);
Value vmNativeFilter(VM& vm, const std::vector<Value>& arguments);
Value vmNativeReduce(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSort(VM& vm, const std::vector<Value>& arguments);
Value vmNativeReverse(VM& vm, const std::vector<Value>& arguments);
Value vmNativeConcat(VM& vm, const std::vector<Value>& arguments);
Value vmNativeSlice(VM& vm, const std::vector<Value>& arguments);

// std.io native functions
Value vmNativeReadFile(VM& vm, const std::vector<Value>& arguments);
Value vmNativeWriteFile(VM& vm, const std::vector<Value>& arguments);
Value vmNativeAppendFile(VM& vm, const std::vector<Value>& arguments);
Value vmNativeFileExists(VM& vm, const std::vector<Value>& arguments);

// std.log native functions
Value vmNativeLogInfo(VM& vm, const std::vector<Value>& arguments);
Value vmNativeLogWarn(VM& vm, const std::vector<Value>& arguments);
Value vmNativeLogError(VM& vm, const std::vector<Value>& arguments);
Value vmNativeLogDebug(VM& vm, const std::vector<Value>& arguments);

// std.time native functions
Value vmNativeTimeNow(VM& vm, const std::vector<Value>& arguments);
Value vmNativeTimeSleep(VM& vm, const std::vector<Value>& arguments);
Value vmNativeTimeFormat(VM& vm, const std::vector<Value>& arguments);

// std.json native functions
Value vmNativeJsonParse(VM& vm, const std::vector<Value>& arguments);
Value vmNativeJsonStringify(VM& vm, const std::vector<Value>& arguments);

// std.regex native functions
Value vmNativeRegexMatch(VM& vm, const std::vector<Value>& arguments);
Value vmNativeRegexReplace(VM& vm, const std::vector<Value>& arguments);
Value vmNativeRegexTest(VM& vm, const std::vector<Value>& arguments);

void registerVmNatives(VM& vm);

}  // namespace izi

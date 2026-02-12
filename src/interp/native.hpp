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
auto nativeShift(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeUnshift(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSplice(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeKeys(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeValues(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeHasKey(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeHas(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeDelete(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeEntries(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSetAdd(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSetHas(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSetDelete(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSetSize(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSet(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.math functions
auto nativeSqrt(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativePow(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeAbs(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeFloor(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeCeil(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeRound(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSin(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeCos(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeTan(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeMin(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeMax(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeClamp(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSign(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeIsFinite(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeIsNaN(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.string functions
auto nativeSubstring(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSplit(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeJoin(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeToUpper(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeToLower(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeTrim(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeReplace(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeStartsWith(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeEndsWith(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeIndexOf(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.array functions
auto nativeMap(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeFilter(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeReduce(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSort(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeReverse(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeConcat(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeSlice(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.io functions
auto nativeReadFile(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeWriteFile(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeAppendFile(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeFileExists(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.log functions
auto nativeLogInfo(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeLogWarn(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeLogError(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeLogDebug(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.assert functions
auto nativeAssertOk(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeAssertEq(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeAssertNe(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.env functions
auto nativeEnvGet(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeEnvSet(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeEnvExists(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.process functions
auto nativeProcessExit(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeProcessStatus(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeProcessArgs(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.path functions
auto nativePathJoin(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativePathBasename(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativePathDirname(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativePathExtname(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativePathNormalize(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.fs functions
auto nativeFsExists(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeFsRead(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeFsWrite(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeFsAppend(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeFsRemove(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.time functions
auto nativeTimeNow(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeTimeSleep(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeTimeFormat(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.json functions
auto nativeJsonParse(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeJsonStringify(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

// std.regex functions
auto nativeRegexMatch(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeRegexReplace(Interpreter& interp, const std::vector<Value>& arguments) -> Value;
auto nativeRegexTest(Interpreter& interp, const std::vector<Value>& arguments) -> Value;

void registerNativeFunctions(Interpreter& interpreter);

}  // namespace izi
#include "native_modules.hpp"
#include "native.hpp"
#include "interpreter.hpp"
#include <cmath>

namespace izi {

Value createMathModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();
    
    // Constants
    module->entries["pi"] = M_PI;
    module->entries["PI"] = M_PI;
    module->entries["e"] = M_E;
    module->entries["E"] = M_E;
    
    // Functions
    module->entries["sqrt"] = Value{std::make_shared<NativeFunction>("sqrt", 1, nativeSqrt)};
    module->entries["pow"] = Value{std::make_shared<NativeFunction>("pow", 2, nativePow)};
    module->entries["abs"] = Value{std::make_shared<NativeFunction>("abs", 1, nativeAbs)};
    module->entries["floor"] = Value{std::make_shared<NativeFunction>("floor", 1, nativeFloor)};
    module->entries["ceil"] = Value{std::make_shared<NativeFunction>("ceil", 1, nativeCeil)};
    module->entries["round"] = Value{std::make_shared<NativeFunction>("round", 1, nativeRound)};
    module->entries["sin"] = Value{std::make_shared<NativeFunction>("sin", 1, nativeSin)};
    module->entries["cos"] = Value{std::make_shared<NativeFunction>("cos", 1, nativeCos)};
    module->entries["tan"] = Value{std::make_shared<NativeFunction>("tan", 1, nativeTan)};
    module->entries["min"] = Value{std::make_shared<NativeFunction>("min", -1, nativeMin)};
    module->entries["max"] = Value{std::make_shared<NativeFunction>("max", -1, nativeMax)};
    module->entries["clamp"] = Value{std::make_shared<NativeFunction>("clamp", 3, nativeClamp)};
    module->entries["sign"] = Value{std::make_shared<NativeFunction>("sign", 1, nativeSign)};
    module->entries["isFinite"] = Value{std::make_shared<NativeFunction>("isFinite", 1, nativeIsFinite)};
    module->entries["isNaN"] = Value{std::make_shared<NativeFunction>("isNaN", 1, nativeIsNaN)};
    
    return Value{module};
}

Value createStringModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();
    
    // String manipulation functions
    module->entries["substring"] = Value{std::make_shared<NativeFunction>("substring", -1, nativeSubstring)};
    module->entries["split"] = Value{std::make_shared<NativeFunction>("split", 2, nativeSplit)};
    module->entries["join"] = Value{std::make_shared<NativeFunction>("join", 2, nativeJoin)};
    module->entries["toUpper"] = Value{std::make_shared<NativeFunction>("toUpper", 1, nativeToUpper)};
    module->entries["toLower"] = Value{std::make_shared<NativeFunction>("toLower", 1, nativeToLower)};
    module->entries["trim"] = Value{std::make_shared<NativeFunction>("trim", 1, nativeTrim)};
    module->entries["replace"] = Value{std::make_shared<NativeFunction>("replace", 3, nativeReplace)};
    module->entries["startsWith"] = Value{std::make_shared<NativeFunction>("startsWith", 2, nativeStartsWith)};
    module->entries["endsWith"] = Value{std::make_shared<NativeFunction>("endsWith", 2, nativeEndsWith)};
    module->entries["indexOf"] = Value{std::make_shared<NativeFunction>("indexOf", 2, nativeIndexOf)};
    
    return Value{module};
}

Value createArrayModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();
    
    // Array manipulation functions
    module->entries["map"] = Value{std::make_shared<NativeFunction>("map", 2, nativeMap)};
    module->entries["filter"] = Value{std::make_shared<NativeFunction>("filter", 2, nativeFilter)};
    module->entries["reduce"] = Value{std::make_shared<NativeFunction>("reduce", -1, nativeReduce)};
    module->entries["sort"] = Value{std::make_shared<NativeFunction>("sort", 1, nativeSort)};
    module->entries["reverse"] = Value{std::make_shared<NativeFunction>("reverse", 1, nativeReverse)};
    module->entries["concat"] = Value{std::make_shared<NativeFunction>("concat", 2, nativeConcat)};
    module->entries["slice"] = Value{std::make_shared<NativeFunction>("slice", -1, nativeSlice)};
    module->entries["push"] = Value{std::make_shared<NativeFunction>("push", 2, nativePush)};
    module->entries["pop"] = Value{std::make_shared<NativeFunction>("pop", 1, nativePop)};
    module->entries["shift"] = Value{std::make_shared<NativeFunction>("shift", 1, nativeShift)};
    module->entries["unshift"] = Value{std::make_shared<NativeFunction>("unshift", 2, nativeUnshift)};
    module->entries["splice"] = Value{std::make_shared<NativeFunction>("splice", -1, nativeSplice)};
    
    return Value{module};
}

Value createIOModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();
    
    // I/O functions
    module->entries["readFile"] = Value{std::make_shared<NativeFunction>("readFile", 1, nativeReadFile)};
    module->entries["writeFile"] = Value{std::make_shared<NativeFunction>("writeFile", 2, nativeWriteFile)};
    module->entries["appendFile"] = Value{std::make_shared<NativeFunction>("appendFile", 2, nativeAppendFile)};
    module->entries["fileExists"] = Value{std::make_shared<NativeFunction>("fileExists", 1, nativeFileExists)};
    module->entries["exists"] = Value{std::make_shared<NativeFunction>("exists", 1, nativeFileExists)};
    
    return Value{module};
}

Value createLogModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();
    
    // Logging functions
    module->entries["info"] = Value{std::make_shared<NativeFunction>("info", 1, nativeLogInfo)};
    module->entries["warn"] = Value{std::make_shared<NativeFunction>("warn", 1, nativeLogWarn)};
    module->entries["error"] = Value{std::make_shared<NativeFunction>("error", 1, nativeLogError)};
    module->entries["debug"] = Value{std::make_shared<NativeFunction>("debug", 1, nativeLogDebug)};
    
    return Value{module};
}

Value createAssertModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();
    
    // Assertion functions
    module->entries["ok"] = Value{std::make_shared<NativeFunction>("ok", -1, nativeAssertOk)};
    module->entries["eq"] = Value{std::make_shared<NativeFunction>("eq", 2, nativeAssertEq)};
    module->entries["ne"] = Value{std::make_shared<NativeFunction>("ne", 2, nativeAssertNe)};
    
    return Value{module};
}

bool isNativeModule(const std::string& path) {
    return path == "math" || path == "string" || path == "array" || 
           path == "io" || path == "json" || path == "http" || 
           path == "log" || path == "std.log" || 
           path == "assert" || path == "std.assert";
}

Value getNativeModule(const std::string& name, Interpreter& interp) {
    if (name == "math") {
        return createMathModule(interp);
    } else if (name == "string") {
        return createStringModule(interp);
    } else if (name == "array") {
        return createArrayModule(interp);
    } else if (name == "io") {
        return createIOModule(interp);
    } else if (name == "log" || name == "std.log") {
        return createLogModule(interp);
    } else if (name == "assert" || name == "std.assert") {
        return createAssertModule(interp);
    } else if (name == "json") {
        // Placeholder for future implementation
        auto module = std::make_shared<Map>();
        return Value{module};
    } else if (name == "http") {
        // Placeholder for future implementation
        auto module = std::make_shared<Map>();
        return Value{module};
    }
    
    throw std::runtime_error("Unknown native module: " + name);
}

}  // namespace izi

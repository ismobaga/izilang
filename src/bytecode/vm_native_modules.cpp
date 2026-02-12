#include "vm_native_modules.hpp"
#include "vm_native.hpp"
#include "vm.hpp"
#include <cmath>

namespace izi {

Value createVmMathModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Constants
    module->entries["pi"] = M_PI;
    module->entries["PI"] = M_PI;
    module->entries["e"] = M_E;
    module->entries["E"] = M_E;
    
    // Functions
    module->entries["sqrt"] = Value{std::make_shared<VmNativeFunction>("sqrt", 1, vmNativeSqrt)};
    module->entries["pow"] = Value{std::make_shared<VmNativeFunction>("pow", 2, vmNativePow)};
    module->entries["abs"] = Value{std::make_shared<VmNativeFunction>("abs", 1, vmNativeAbs)};
    module->entries["floor"] = Value{std::make_shared<VmNativeFunction>("floor", 1, vmNativeFloor)};
    module->entries["ceil"] = Value{std::make_shared<VmNativeFunction>("ceil", 1, vmNativeCeil)};
    module->entries["round"] = Value{std::make_shared<VmNativeFunction>("round", 1, vmNativeRound)};
    module->entries["sin"] = Value{std::make_shared<VmNativeFunction>("sin", 1, vmNativeSin)};
    module->entries["cos"] = Value{std::make_shared<VmNativeFunction>("cos", 1, vmNativeCos)};
    module->entries["tan"] = Value{std::make_shared<VmNativeFunction>("tan", 1, vmNativeTan)};
    module->entries["min"] = Value{std::make_shared<VmNativeFunction>("min", -1, vmNativeMin)};
    module->entries["max"] = Value{std::make_shared<VmNativeFunction>("max", -1, vmNativeMax)};
    
    return Value{module};
}

Value createVmStringModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // String manipulation functions
    module->entries["substring"] = Value{std::make_shared<VmNativeFunction>("substring", -1, vmNativeSubstring)};
    module->entries["split"] = Value{std::make_shared<VmNativeFunction>("split", 2, vmNativeSplit)};
    module->entries["join"] = Value{std::make_shared<VmNativeFunction>("join", 2, vmNativeJoin)};
    module->entries["toUpper"] = Value{std::make_shared<VmNativeFunction>("toUpper", 1, vmNativeToUpper)};
    module->entries["toLower"] = Value{std::make_shared<VmNativeFunction>("toLower", 1, vmNativeToLower)};
    module->entries["trim"] = Value{std::make_shared<VmNativeFunction>("trim", 1, vmNativeTrim)};
    module->entries["replace"] = Value{std::make_shared<VmNativeFunction>("replace", 3, vmNativeReplace)};
    module->entries["startsWith"] = Value{std::make_shared<VmNativeFunction>("startsWith", 2, vmNativeStartsWith)};
    module->entries["endsWith"] = Value{std::make_shared<VmNativeFunction>("endsWith", 2, vmNativeEndsWith)};
    module->entries["indexOf"] = Value{std::make_shared<VmNativeFunction>("indexOf", 2, vmNativeIndexOf)};
    
    return Value{module};
}

Value createVmArrayModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Array manipulation functions
    module->entries["map"] = Value{std::make_shared<VmNativeFunction>("map", 2, vmNativeMap)};
    module->entries["filter"] = Value{std::make_shared<VmNativeFunction>("filter", 2, vmNativeFilter)};
    module->entries["reduce"] = Value{std::make_shared<VmNativeFunction>("reduce", -1, vmNativeReduce)};
    module->entries["sort"] = Value{std::make_shared<VmNativeFunction>("sort", 1, vmNativeSort)};
    module->entries["reverse"] = Value{std::make_shared<VmNativeFunction>("reverse", 1, vmNativeReverse)};
    module->entries["concat"] = Value{std::make_shared<VmNativeFunction>("concat", 2, vmNativeConcat)};
    module->entries["slice"] = Value{std::make_shared<VmNativeFunction>("slice", -1, vmNativeSlice)};
    module->entries["push"] = Value{std::make_shared<VmNativeFunction>("push", 2, vmNativePush)};
    module->entries["pop"] = Value{std::make_shared<VmNativeFunction>("pop", 1, vmNativePop)};
    module->entries["shift"] = Value{std::make_shared<VmNativeFunction>("shift", 1, vmNativeShift)};
    module->entries["unshift"] = Value{std::make_shared<VmNativeFunction>("unshift", 2, vmNativeUnshift)};
    module->entries["splice"] = Value{std::make_shared<VmNativeFunction>("splice", -1, vmNativeSplice)};
    
    return Value{module};
}

Value createVmIOModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // I/O functions
    module->entries["readFile"] = Value{std::make_shared<VmNativeFunction>("readFile", 1, vmNativeReadFile)};
    module->entries["writeFile"] = Value{std::make_shared<VmNativeFunction>("writeFile", 2, vmNativeWriteFile)};
    module->entries["appendFile"] = Value{std::make_shared<VmNativeFunction>("appendFile", 2, vmNativeAppendFile)};
    module->entries["fileExists"] = Value{std::make_shared<VmNativeFunction>("fileExists", 1, vmNativeFileExists)};
    module->entries["exists"] = Value{std::make_shared<VmNativeFunction>("exists", 1, vmNativeFileExists)};
    
    return Value{module};
}

Value createVmLogModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Logging functions
    module->entries["info"] = Value{std::make_shared<VmNativeFunction>("info", 1, vmNativeLogInfo)};
    module->entries["warn"] = Value{std::make_shared<VmNativeFunction>("warn", 1, vmNativeLogWarn)};
    module->entries["error"] = Value{std::make_shared<VmNativeFunction>("error", 1, vmNativeLogError)};
    module->entries["debug"] = Value{std::make_shared<VmNativeFunction>("debug", 1, vmNativeLogDebug)};
    
    return Value{module};
}

Value createVmAssertModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Assertion functions (placeholder - need VM versions)
    // module->entries["ok"] = Value{std::make_shared<VmNativeFunction>("ok", -1, vmNativeAssertOk)};
    // module->entries["eq"] = Value{std::make_shared<VmNativeFunction>("eq", 2, vmNativeAssertEq)};
    // module->entries["ne"] = Value{std::make_shared<VmNativeFunction>("ne", 2, vmNativeAssertNe)};
    
    return Value{module};
}

Value createVmEnvModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Environment variable functions (placeholder - need VM versions)
    // module->entries["get"] = Value{std::make_shared<VmNativeFunction>("get", 1, vmNativeEnvGet)};
    // module->entries["set"] = Value{std::make_shared<VmNativeFunction>("set", 2, vmNativeEnvSet)};
    // module->entries["exists"] = Value{std::make_shared<VmNativeFunction>("exists", 1, vmNativeEnvExists)};
    
    return Value{module};
}

Value createVmProcessModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Process control functions (placeholder - need VM versions)
    // module->entries["exit"] = Value{std::make_shared<VmNativeFunction>("exit", 1, vmNativeProcessExit)};
    // module->entries["status"] = Value{std::make_shared<VmNativeFunction>("status", 0, vmNativeProcessStatus)};
    // module->entries["args"] = Value{std::make_shared<VmNativeFunction>("args", 0, vmNativeProcessArgs)};
    
    return Value{module};
}

Value createVmPathModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Path manipulation functions (placeholder - need VM versions)
    // module->entries["join"] = Value{std::make_shared<VmNativeFunction>("join", -1, vmNativePathJoin)};
    // module->entries["basename"] = Value{std::make_shared<VmNativeFunction>("basename", 1, vmNativePathBasename)};
    // module->entries["dirname"] = Value{std::make_shared<VmNativeFunction>("dirname", 1, vmNativePathDirname)};
    // module->entries["extname"] = Value{std::make_shared<VmNativeFunction>("extname", 1, vmNativePathExtname)};
    // module->entries["normalize"] = Value{std::make_shared<VmNativeFunction>("normalize", 1, vmNativePathNormalize)};
    
    return Value{module};
}

Value createVmFsModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Filesystem functions (placeholder - need VM versions)
    // module->entries["exists"] = Value{std::make_shared<VmNativeFunction>("exists", 1, vmNativeFsExists)};
    // module->entries["read"] = Value{std::make_shared<VmNativeFunction>("read", 1, vmNativeFsRead)};
    // module->entries["write"] = Value{std::make_shared<VmNativeFunction>("write", 2, vmNativeFsWrite)};
    // module->entries["append"] = Value{std::make_shared<VmNativeFunction>("append", 2, vmNativeFsAppend)};
    // module->entries["remove"] = Value{std::make_shared<VmNativeFunction>("remove", 1, vmNativeFsRemove)};
    
    return Value{module};
}

Value createVmTimeModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Time functions
    module->entries["now"] = Value{std::make_shared<VmNativeFunction>("now", 0, vmNativeTimeNow)};
    module->entries["sleep"] = Value{std::make_shared<VmNativeFunction>("sleep", 1, vmNativeTimeSleep)};
    module->entries["format"] = Value{std::make_shared<VmNativeFunction>("format", -1, vmNativeTimeFormat)};
    
    return Value{module};
}

Value createVmJsonModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // JSON functions
    module->entries["parse"] = Value{std::make_shared<VmNativeFunction>("parse", 1, vmNativeJsonParse)};
    module->entries["stringify"] = Value{std::make_shared<VmNativeFunction>("stringify", 1, vmNativeJsonStringify)};
    
    return Value{module};
}

Value createVmRegexModule(VM& vm) {
    auto module = std::make_shared<Map>();
    
    // Regex functions
    module->entries["match"] = Value{std::make_shared<VmNativeFunction>("match", 2, vmNativeRegexMatch)};
    module->entries["replace"] = Value{std::make_shared<VmNativeFunction>("replace", 3, vmNativeRegexReplace)};
    module->entries["test"] = Value{std::make_shared<VmNativeFunction>("test", 2, vmNativeRegexTest)};
    
    return Value{module};
}

bool isVmNativeModule(const std::string& path) {
    return path == "math" || path == "string" || path == "array" || 
           path == "io" || path == "json" || path == "std.json" ||
           path == "http" || 
           path == "log" || path == "std.log" || 
           path == "assert" || path == "std.assert" ||
           path == "env" || path == "std.env" ||
           path == "process" || path == "std.process" ||
           path == "path" || path == "std.path" ||
           path == "fs" || path == "std.fs" ||
           path == "time" || path == "std.time" ||
           path == "regex" || path == "std.regex";
}

Value getVmNativeModule(const std::string& name, VM& vm) {
    if (name == "math") {
        return createVmMathModule(vm);
    } else if (name == "string") {
        return createVmStringModule(vm);
    } else if (name == "array") {
        return createVmArrayModule(vm);
    } else if (name == "io") {
        return createVmIOModule(vm);
    } else if (name == "log" || name == "std.log") {
        return createVmLogModule(vm);
    } else if (name == "assert" || name == "std.assert") {
        return createVmAssertModule(vm);
    } else if (name == "env" || name == "std.env") {
        return createVmEnvModule(vm);
    } else if (name == "process" || name == "std.process") {
        return createVmProcessModule(vm);
    } else if (name == "path" || name == "std.path") {
        return createVmPathModule(vm);
    } else if (name == "fs" || name == "std.fs") {
        return createVmFsModule(vm);
    } else if (name == "time" || name == "std.time") {
        return createVmTimeModule(vm);
    } else if (name == "json" || name == "std.json") {
        return createVmJsonModule(vm);
    } else if (name == "regex" || name == "std.regex") {
        return createVmRegexModule(vm);
    } else if (name == "http") {
        // Placeholder for future implementation
        auto module = std::make_shared<Map>();
        return Value{module};
    }
    
    throw std::runtime_error("Unknown native module: " + name);
}

}  // namespace izi

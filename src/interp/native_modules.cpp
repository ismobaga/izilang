#include "native_modules.hpp"
#include "native.hpp"
#include "native_ui.hpp"
#include "native_audio.hpp"
#include "native_image.hpp"
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

Value createEnvModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // Environment variable functions
    module->entries["get"] = Value{std::make_shared<NativeFunction>("get", 1, nativeEnvGet)};
    module->entries["set"] = Value{std::make_shared<NativeFunction>("set", 2, nativeEnvSet)};
    module->entries["exists"] = Value{std::make_shared<NativeFunction>("exists", 1, nativeEnvExists)};

    return Value{module};
}

Value createProcessModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // Process control functions
    module->entries["exit"] = Value{std::make_shared<NativeFunction>("exit", 1, nativeProcessExit)};
    module->entries["status"] = Value{std::make_shared<NativeFunction>("status", 0, nativeProcessStatus)};
    module->entries["args"] = Value{std::make_shared<NativeFunction>("args", 0, nativeProcessArgs)};

    return Value{module};
}

Value createPathModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // Path manipulation functions
    module->entries["join"] = Value{std::make_shared<NativeFunction>("join", -1, nativePathJoin)};
    module->entries["basename"] = Value{std::make_shared<NativeFunction>("basename", 1, nativePathBasename)};
    module->entries["dirname"] = Value{std::make_shared<NativeFunction>("dirname", 1, nativePathDirname)};
    module->entries["extname"] = Value{std::make_shared<NativeFunction>("extname", 1, nativePathExtname)};
    module->entries["normalize"] = Value{std::make_shared<NativeFunction>("normalize", 1, nativePathNormalize)};

    return Value{module};
}

Value createFsModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // Filesystem functions
    module->entries["exists"] = Value{std::make_shared<NativeFunction>("exists", 1, nativeFsExists)};
    module->entries["read"] = Value{std::make_shared<NativeFunction>("read", 1, nativeFsRead)};
    module->entries["write"] = Value{std::make_shared<NativeFunction>("write", 2, nativeFsWrite)};
    module->entries["append"] = Value{std::make_shared<NativeFunction>("append", 2, nativeFsAppend)};
    module->entries["remove"] = Value{std::make_shared<NativeFunction>("remove", 1, nativeFsRemove)};

    return Value{module};
}

Value createHttpModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // HTTP client functions
    module->entries["get"] = Value{std::make_shared<NativeFunction>("get", 1, nativeHttpGet)};
    // post accepts 2 required args (url, body) and 1 optional arg (contentType)
    module->entries["post"] = Value{std::make_shared<NativeFunction>("post", -1, nativeHttpPost)};
    module->entries["request"] = Value{std::make_shared<NativeFunction>("request", 1, nativeHttpRequest)};

    return Value{module};
}

Value createNetModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // TCP client/server functions
    module->entries["connect"]    = Value{std::make_shared<NativeFunction>("connect", 2, nativeNetConnect)};
    module->entries["listen"]     = Value{std::make_shared<NativeFunction>("listen", 1, nativeNetListen)};
    module->entries["accept"]     = Value{std::make_shared<NativeFunction>("accept", -1, nativeNetAccept)};
    module->entries["send"]       = Value{std::make_shared<NativeFunction>("send", 2, nativeNetSend)};
    module->entries["recv"]       = Value{std::make_shared<NativeFunction>("recv", -1, nativeNetRecv)};
    module->entries["close"]      = Value{std::make_shared<NativeFunction>("close", 1, nativeNetClose)};
    module->entries["setTimeout"] = Value{std::make_shared<NativeFunction>("setTimeout", 2, nativeNetSetTimeout)};

    return Value{module};
}

Value createIpcModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // IPC functions
    module->entries["createPipe"] = Value{std::make_shared<NativeFunction>("createPipe", 1, nativeIpcCreatePipe)};
    module->entries["openRead"]   = Value{std::make_shared<NativeFunction>("openRead", 1, nativeIpcOpenRead)};
    module->entries["openWrite"]  = Value{std::make_shared<NativeFunction>("openWrite", 1, nativeIpcOpenWrite)};
    module->entries["send"]       = Value{std::make_shared<NativeFunction>("send", 2, nativeIpcSend)};
    module->entries["recv"]       = Value{std::make_shared<NativeFunction>("recv", 1, nativeIpcRecv)};
    module->entries["tryRecv"]    = Value{std::make_shared<NativeFunction>("tryRecv", 1, nativeIpcTryRecv)};
    module->entries["close"]      = Value{std::make_shared<NativeFunction>("close", 1, nativeIpcClose)};
    module->entries["removePipe"] = Value{std::make_shared<NativeFunction>("removePipe", 1, nativeIpcRemovePipe)};

    return Value{module};
}

Value createTimeModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // Time functions
    module->entries["now"] = Value{std::make_shared<NativeFunction>("now", 0, nativeTimeNow)};
    module->entries["sleep"] = Value{std::make_shared<NativeFunction>("sleep", 1, nativeTimeSleep)};
    module->entries["format"] = Value{std::make_shared<NativeFunction>("format", -1, nativeTimeFormat)};

    return Value{module};
}

Value createJsonModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // JSON functions
    module->entries["parse"] = Value{std::make_shared<NativeFunction>("parse", 1, nativeJsonParse)};
    module->entries["stringify"] = Value{std::make_shared<NativeFunction>("stringify", 1, nativeJsonStringify)};

    return Value{module};
}

Value createRegexModule(Interpreter& interp) {
    auto module = std::make_shared<Map>();

    // Regex functions
    module->entries["match"] = Value{std::make_shared<NativeFunction>("match", 2, nativeRegexMatch)};
    module->entries["replace"] = Value{std::make_shared<NativeFunction>("replace", 3, nativeRegexReplace)};
    module->entries["test"] = Value{std::make_shared<NativeFunction>("test", 2, nativeRegexTest)};

    return Value{module};
}

bool isNativeModule(const std::string& path) {
    return path == "math" || path == "std.math" || path == "string" || path == "std.string" ||
           path == "array" || path == "std.array" || path == "io" || path == "std.io" || path == "json" ||
           path == "std.json" || path == "http" || path == "std.http" || path == "log" || path == "std.log" ||
           path == "assert" || path == "std.assert" || path == "env" || path == "std.env" || path == "process" ||
           path == "std.process" || path == "path" || path == "std.path" || path == "fs" || path == "std.fs" ||
           path == "time" || path == "std.time" || path == "regex" || path == "std.regex" ||
           path == "ui" || path == "std.ui" || path == "audio" || path == "std.audio" ||
           path == "image" || path == "std.image" ||
           path == "ipc" || path == "std.ipc" ||
           path == "net" || path == "std.net";
}

Value getNativeModule(const std::string& name, Interpreter& interp) {
    if (name == "math" || name == "std.math") {
        return createMathModule(interp);
    } else if (name == "string" || name == "std.string") {
        return createStringModule(interp);
    } else if (name == "array" || name == "std.array") {
        return createArrayModule(interp);
    } else if (name == "io" || name == "std.io") {
        return createIOModule(interp);
    } else if (name == "log" || name == "std.log") {
        return createLogModule(interp);
    } else if (name == "assert" || name == "std.assert") {
        return createAssertModule(interp);
    } else if (name == "env" || name == "std.env") {
        return createEnvModule(interp);
    } else if (name == "process" || name == "std.process") {
        return createProcessModule(interp);
    } else if (name == "path" || name == "std.path") {
        return createPathModule(interp);
    } else if (name == "fs" || name == "std.fs") {
        return createFsModule(interp);
    } else if (name == "time" || name == "std.time") {
        return createTimeModule(interp);
    } else if (name == "json" || name == "std.json") {
        return createJsonModule(interp);
    } else if (name == "regex" || name == "std.regex") {
        return createRegexModule(interp);
    } else if (name == "http" || name == "std.http") {
        return createHttpModule(interp);
    } else if (name == "ui" || name == "std.ui") {
        return createUiModule(interp);
    } else if (name == "audio" || name == "std.audio") {
        return createAudioModule(interp);
    } else if (name == "image" || name == "std.image") {
        return createImageModule(interp);
    } else if (name == "ipc" || name == "std.ipc") {
        return createIpcModule(interp);
    } else if (name == "net" || name == "std.net") {
        return createNetModule(interp);
    }

    throw std::runtime_error("Unknown native module: " + name);
}

}  // namespace izi

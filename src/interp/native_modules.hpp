#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "../common/value.hpp"

namespace izi {

class Interpreter;

// Helper to create a native module (returns a Map Value)
Value createMathModule(Interpreter& interp);
Value createStringModule(Interpreter& interp);
Value createArrayModule(Interpreter& interp);
Value createIOModule(Interpreter& interp);
Value createAssertModule(Interpreter& interp);
Value createEnvModule(Interpreter& interp);
Value createProcessModule(Interpreter& interp);

// Check if a module path refers to a native module
bool isNativeModule(const std::string& path);

// Get a native module by name
Value getNativeModule(const std::string& name, Interpreter& interp);

}  // namespace izi

#pragma once

#include "common/value.hpp"
#include "bytecode/vm.hpp"
#include <string>

namespace izi {

// Check if a module path refers to a native module
bool isVmNativeModule(const std::string& path);

// Get a native module by name
Value getVmNativeModule(const std::string& name, VM& vm);

// Create the ui native module for the bytecode VM
Value createVmUiModule(VM& vm);

// Create the audio native module for the bytecode VM
Value createVmAudioModule(VM& vm);

// Create the image native module for the bytecode VM
Value createVmImageModule(VM& vm);

}  // namespace izi

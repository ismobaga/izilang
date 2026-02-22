#pragma once

#include "../common/value.hpp"

namespace izi {

class VM;

// Create the image native module for the bytecode VM
Value createVmImageModule(VM& vm);

}  // namespace izi

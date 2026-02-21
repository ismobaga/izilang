#pragma once

#include "../common/value.hpp"

namespace izi {

class VM;

// Create the ui native module for the bytecode VM
Value createVmUiModule(VM& vm);

}  // namespace izi

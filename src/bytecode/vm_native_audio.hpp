#pragma once

#include "../common/value.hpp"

namespace izi {

class VM;

// Create the audio native module for the bytecode VM
Value createVmAudioModule(VM& vm);

}  // namespace izi

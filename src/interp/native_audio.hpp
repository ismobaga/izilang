#pragma once

#include "../common/value.hpp"

namespace izi {

class Interpreter;

// Create the audio native module
Value createAudioModule(Interpreter& interp);

}  // namespace izi

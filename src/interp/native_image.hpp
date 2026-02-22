#pragma once

#include "../common/value.hpp"

namespace izi {

class Interpreter;

// Create the image native module
Value createImageModule(Interpreter& interp);

}  // namespace izi

#pragma once

#include "../common/value.hpp"

namespace izi {

class Interpreter;

// Create the ui native module
Value createUiModule(Interpreter& interp);

}  // namespace izi

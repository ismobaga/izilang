#pragma once
#include <vector>

#include "value.hpp"

namespace izi {
class Interpreter;  // forward

class Callable {
   public:
    virtual ~Callable() = default;
    virtual int arity() const = 0;
    virtual Value call(Interpreter& interp, const std::vector<Value>& args) = 0;
    virtual std::string name() const = 0;
};

}  // namespace izi

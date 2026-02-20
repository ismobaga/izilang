#pragma once

#include <memory>
#include <string>
#include <vector>

#include "common/value.hpp"

namespace izi {
class VM;
class VmCallable {
   public:
    virtual ~VmCallable() = default;

    virtual std::string name() const = 0;
    virtual int arity() const = 0;  // -1 for variadic
    virtual Value call(VM& vm, const std::vector<Value>& args) = 0;
};
}  // namespace izi
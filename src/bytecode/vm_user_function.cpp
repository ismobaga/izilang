#include "vm_user_function.hpp"
#include "vm.hpp"

namespace izi {

Value VmUserFunction::call(VM& vm, const std::vector<Value>& arguments) {
    // Execute the function's bytecode, passing arguments as initial local variable slots.
    // Each argument is pushed onto the VM stack immediately after the call frame is
    // created, so GET_LOCAL 0 == first parameter, GET_LOCAL 1 == second parameter, etc.
    return vm.run(*chunk_, arguments);
}

}  // namespace izi

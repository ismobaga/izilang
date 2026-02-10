#include "vm_user_function.hpp"
#include "vm.hpp"

namespace izi {

Value VmUserFunction::call(VM& vm, const std::vector<Value>& arguments) {
    // Set parameters as globals before executing function
    for (size_t i = 0; i < params_.size(); ++i) {
        vm.setGlobal(params_[i], arguments[i]);
    }
    
    // Execute the function's bytecode
    return vm.run(*chunk_);
}

} // namespace izi

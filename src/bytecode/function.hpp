#pragma once

#include "chunk.hpp"
#include <memory>
#include <string>
#include <vector>

namespace izi {

// Function: a compiled unit consisting of a bytecode Chunk together with its
// name and parameter list.  This is the core data structure that the VM
// executes when calling user-defined functions.
struct Function {
    std::string name;
    std::vector<std::string> params;
    std::shared_ptr<Chunk> chunk;

    Function() : chunk(std::make_shared<Chunk>()) {}
    Function(std::string n, std::vector<std::string> p, std::shared_ptr<Chunk> c)
        : name(std::move(n)), params(std::move(p)), chunk(std::move(c)) {}

    int arity() const { return static_cast<int>(params.size()); }
};

}  // namespace izi

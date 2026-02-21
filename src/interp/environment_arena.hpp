#pragma once

#include <memory>
#include <vector>

namespace izi {

class Environment;

// Arena allocator for Environment objects.
//
// Ownership model:
//   The EnvironmentArena is the sole owner of all Environment instances created
//   through it.  Callers receive raw (non-owning) pointers and must not delete
//   them.  All environments are freed when the arena is destroyed (typically
//   when the Interpreter that owns the arena goes out of scope).
//
// This eliminates the shared_ptr reference cycle that would otherwise exist
// between UserFunction (which captures a closure environment) and Environment
// (which can store Values that are UserFunction objects).  Because the arena
// outlives every environment it allocates, raw pointers are always valid for
// the lifetime of the interpreter.
class EnvironmentArena {
   public:
    // Create a root environment (no parent).
    Environment* create();

    // Create a child environment whose parent is `parent`.
    Environment* create(Environment* parent);

    // Release all environments owned by this arena.
    void reset() { envs_.clear(); }

    size_t size() const { return envs_.size(); }

   private:
    std::vector<std::unique_ptr<Environment>> envs_;
};

}  // namespace izi

// Include after the declaration so that EnvironmentArena::create() can
// construct Environment objects.
#include "environment.hpp"

namespace izi {

inline Environment* EnvironmentArena::create() {
    envs_.push_back(std::make_unique<Environment>());
    return envs_.back().get();
}

inline Environment* EnvironmentArena::create(Environment* parent) {
    envs_.push_back(std::make_unique<Environment>(parent));
    return envs_.back().get();
}

}  // namespace izi

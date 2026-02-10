#pragma once

#include "mv_callable.hpp"
#include "chunk.hpp"
#include <memory>
#include <vector>
#include <string>

namespace izi {

class VmUserFunction : public VmCallable {
public:
    VmUserFunction(std::string name, std::vector<std::string> params, std::shared_ptr<Chunk> functionChunk)
        : name_(std::move(name)), params_(std::move(params)), chunk_(std::move(functionChunk)) {}

    std::string name() const override { return name_; }
    int arity() const override { return static_cast<int>(params_.size()); }

    Value call(VM& vm, const std::vector<Value>& arguments) override;

    const Chunk& getChunk() const { return *chunk_; }
    const std::vector<std::string>& params() const { return params_; }

private:
    std::string name_;
    std::vector<std::string> params_;
    std::shared_ptr<Chunk> chunk_;
};

} // namespace izi

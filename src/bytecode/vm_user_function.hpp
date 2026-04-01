#pragma once

#include "mv_callable.hpp"
#include "chunk.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace izi {

class VmUserFunction : public VmCallable, public std::enable_shared_from_this<VmUserFunction> {
     public:
        VmUserFunction(std::string name, std::vector<std::string> params, std::shared_ptr<Chunk> functionChunk,
                                     std::vector<std::string> localNames = {}, std::vector<std::string> captureNames = {})
                : name_(std::move(name)),
                    params_(std::move(params)),
                    chunk_(std::move(functionChunk)),
                    localNames_(std::move(localNames)),
                    captureNames_(std::move(captureNames)) {}

        VmUserFunction(std::string name, std::vector<std::string> params, std::shared_ptr<Chunk> functionChunk,
                                     std::vector<std::string> localNames, std::vector<std::string> captureNames,
                                     std::unordered_map<std::string, Value> capturedVars)
                : name_(std::move(name)),
                    params_(std::move(params)),
                    chunk_(std::move(functionChunk)),
                    localNames_(std::move(localNames)),
                    captureNames_(std::move(captureNames)),
                    capturedVars_(std::move(capturedVars)) {}

    std::string name() const override { return name_; }
    int arity() const override { return static_cast<int>(params_.size()); }

    Value call(VM& vm, const std::vector<Value>& arguments) override;

    const Chunk& getChunk() const { return *chunk_; }
    const std::vector<std::string>& params() const { return params_; }
    const std::vector<std::string>& localNames() const { return localNames_; }
    const std::vector<std::string>& captureNames() const { return captureNames_; }
    
    // Get captured variable value (returns nullptr if not found)
    const Value* getCapturedVar(const std::string& name) const {
        auto it = capturedVars_.find(name);
        if (it != capturedVars_.end()) {
            return &it->second;
        }
        return nullptr;
    }

    bool setCapturedVar(const std::string& name, const Value& value) {
        auto it = capturedVars_.find(name);
        if (it == capturedVars_.end()) {
            return false;
        }
        it->second = value;
        return true;
    }

    std::shared_ptr<VmUserFunction> bindCaptured(std::unordered_map<std::string, Value> capturedVars) const {
        return std::make_shared<VmUserFunction>(name_, params_, chunk_, localNames_, captureNames_,
                                                std::move(capturedVars));
    }

   private:
    std::string name_;
    std::vector<std::string> params_;
    std::shared_ptr<Chunk> chunk_;
    std::vector<std::string> localNames_;
    std::vector<std::string> captureNames_;
    std::unordered_map<std::string, Value> capturedVars_;
};

}  // namespace izi

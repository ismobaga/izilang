#pragma once

#include "compile/backend.hpp"

namespace izi {

class CompilePipeline {
   public:
    explicit CompilePipeline(std::unique_ptr<CompileBackend> backend);

    BackendArtifact compile(const CompileRequest& request);
    const char* backendName() const;

   private:
    std::unique_ptr<CompileBackend> backend_;
};

}  // namespace izi

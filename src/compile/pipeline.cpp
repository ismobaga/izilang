#include "compile/pipeline.hpp"

#include <stdexcept>

namespace izi {

CompilePipeline::CompilePipeline(std::unique_ptr<CompileBackend> backend) : backend_(std::move(backend)) {
    if (!backend_) {
        throw std::runtime_error("Compile pipeline requires a backend.");
    }
}

Chunk CompilePipeline::compile(const CompileRequest& request) {
    return backend_->compile(request);
}

const char* CompilePipeline::backendName() const {
    return backend_->name();
}

}  // namespace izi

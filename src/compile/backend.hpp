#pragma once

#include "ast/stmt.hpp"
#include "bytecode/chunk.hpp"

#include <memory>
#include <string>
#include <unordered_set>

namespace izi {

enum class BackendTarget {
    VmBytecode,
    LlvmNative,
};

struct CompileRequest {
    const std::vector<StmtPtr>* program = nullptr;
    std::string currentFile;
    std::unordered_set<std::string>* importedModules = nullptr;
};

class CompileBackend {
   public:
    virtual ~CompileBackend() = default;

    virtual Chunk compile(const CompileRequest& request) = 0;
    virtual const char* name() const = 0;
};

std::unique_ptr<CompileBackend> makeBackend(BackendTarget target);

}  // namespace izi

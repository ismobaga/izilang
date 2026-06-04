#pragma once

#include "ast/stmt.hpp"
#include "bytecode/chunk.hpp"

#include <memory>
#include <string>
#include <unordered_set>
#include <variant>

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

struct LlvmModuleArtifact {
    std::string moduleName;
};

using BackendArtifact = std::variant<Chunk, LlvmModuleArtifact>;

class CompileBackend {
   public:
    virtual ~CompileBackend() = default;

    virtual BackendArtifact compile(const CompileRequest& request) = 0;
    virtual const char* name() const = 0;
};

std::unique_ptr<CompileBackend> makeBackend(BackendTarget target);

}  // namespace izi

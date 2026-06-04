#include "compile/backend.hpp"

#include "compile/compiler.hpp"

#include <stdexcept>

namespace izi {

namespace {

class VmBytecodeBackend final : public CompileBackend {
   public:
    BackendArtifact compile(const CompileRequest& request) override {
        if (request.program == nullptr) {
            throw std::runtime_error("Compile request missing program.");
        }

        BytecodeCompiler compiler;
        if (!request.currentFile.empty()) {
            compiler.setCurrentFile(request.currentFile);
        }
        compiler.setImportedModules(request.importedModules);
        return compiler.compile(*request.program);
    }

    const char* name() const override { return "vm-bytecode"; }
};

class LlvmNativeBackend final : public CompileBackend {
   public:
    BackendArtifact compile(const CompileRequest& request) override {
        (void)request;
        throw std::runtime_error("LLVM native backend scaffolding is present but codegen is not implemented yet.");
    }

    const char* name() const override { return "llvm-native"; }
};

}  // namespace

std::unique_ptr<CompileBackend> makeBackend(BackendTarget target) {
    switch (target) {
        case BackendTarget::VmBytecode:
            return std::make_unique<VmBytecodeBackend>();
        case BackendTarget::LlvmNative:
            return std::make_unique<LlvmNativeBackend>();
    }

    throw std::runtime_error("Unknown compile backend target.");
}

}  // namespace izi

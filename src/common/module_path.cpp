#include "module_path.hpp"
#include <stdexcept>

namespace izi {

bool ModulePath::isRelativePath(const std::string& path) {
    if (path.empty()) return false;

    // Check for ./ at the start
    if (path.size() >= 2 && path[0] == '.' && path[1] == '/') {
        return true;
    }

    // Check for ../ at the start
    if (path.size() >= 3 && path[0] == '.' && path[1] == '.' && path[2] == '/') {
        return true;
    }

    return false;
}

std::string ModulePath::normalizeExtension(const std::string& path) {
    // Check if already has .iz or .izi extension
    if (path.size() >= 3 && path.ends_with(".iz")) {
        return path;
    }
    if (path.size() >= 4 && path.ends_with(".izi")) {
        return path;
    }

    // Add .izi extension
    return path + ".izi";
}

std::string ModulePath::getDirectory(const std::string& filePath) {
    namespace fs = std::filesystem;

    if (filePath.empty() || filePath == "<stdin>" || filePath == "<repl>") {
        // For stdin/repl, use current working directory
        return fs::current_path().string();
    }

    fs::path p(filePath);
    if (p.has_parent_path()) {
        return p.parent_path().string();
    }

    // If no parent path, return current directory
    return fs::current_path().string();
}

std::string ModulePath::canonicalize(const std::string& path) {
    namespace fs = std::filesystem;

    try {
        // Try to get the canonical (absolute) path
        // This resolves .., ., and symlinks
        if (fs::exists(path)) {
            return fs::canonical(path).string();
        }

        // If file doesn't exist yet, use weakly_canonical which doesn't require existence
        return fs::weakly_canonical(path).string();
    } catch (const fs::filesystem_error& e) {
        // If canonicalization fails, return the path as-is
        return path;
    }
}

std::string ModulePath::resolveImport(const std::string& importPath, const std::string& importingFile) {
    namespace fs = std::filesystem;

    // First normalize the extension
    std::string pathWithExt = normalizeExtension(importPath);

    // If it's a relative path, resolve it relative to the importing file's directory
    if (isRelativePath(importPath)) {
        std::string importingDir = getDirectory(importingFile);
        fs::path basePath(importingDir);
        fs::path modulePath = basePath / pathWithExt;

        // Normalize the path (resolve .. and .)
        return modulePath.lexically_normal().string();
    }

    // For absolute paths or non-relative paths, return as-is
    // (They will be resolved relative to CWD when opened)
    return pathWithExt;
}

}  // namespace izi

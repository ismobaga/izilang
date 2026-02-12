#pragma once

#include <string>
#include <filesystem>

namespace izi {

// Module path resolution utilities
class ModulePath {
public:
    // Resolve a module path relative to an importing file
    // Examples:
    //   resolveImport("./lib/math", "/home/user/app/main.izi") -> "/home/user/app/lib/math.izi"
    //   resolveImport("../util", "/home/user/app/lib/foo.izi") -> "/home/user/app/util.izi"
    //   resolveImport("math", "/home/user/app/main.izi") -> "math.izi" (relative to CWD)
    static std::string resolveImport(const std::string& importPath, const std::string& importingFile);
    
    // Check if a path is a relative path (starts with ./ or ../)
    static bool isRelativePath(const std::string& path);
    
    // Normalize a module path by adding .izi/.iz extension if needed
    static std::string normalizeExtension(const std::string& path);
    
    // Get the directory containing a file
    static std::string getDirectory(const std::string& filePath);
    
    // Canonicalize a path (resolve .., ., and symlinks)
    static std::string canonicalize(const std::string& path);
};

}  // namespace izi

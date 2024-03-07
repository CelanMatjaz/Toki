#include "directory.h"

namespace Toki {

std::filesystem::path Directory::workingDirectory() {
    return std::filesystem::current_path();
}

std::filesystem::path Directory::cacheDirectory() {
    return workingDirectory() / "cache";
}

std::filesystem::path Directory::assetsDirectory() {
    return workingDirectory() / "assets";
}

std::filesystem::path Directory::shadersDirectory() {
    return assetsDirectory() / "shaders";
}

}  // namespace Toki

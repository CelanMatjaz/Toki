#include "resource_utils.h"

namespace Toki {

bool ResourceUtils::pathExists(std::filesystem::path path) {
    if (path.is_absolute()) {
        return std::filesystem::exists(path);
    } else {
        return std::filesystem::exists(std::filesystem::current_path() / path);
    }
}

bool ResourceUtils::directoryExists(std::filesystem::path path) {
    if (!pathExists(path)) {
        return false;
    }

    return std::filesystem::is_directory(path);
}

bool ResourceUtils::fileExists(std::filesystem::path path) {
    if (!pathExists(path)) {
        return false;
    }

    return !std::filesystem::is_directory(path);
}

void ResourceUtils::ensureDirectory(std::filesystem::path path) {
    if (pathExists(path)) {
        return;
    }
    std::filesystem::create_directory(path);
}

void ResourceUtils::setWorkingDirectory(std::filesystem::path path) {
    std::filesystem::current_path(std::filesystem::absolute(path));
}

}  // namespace Toki
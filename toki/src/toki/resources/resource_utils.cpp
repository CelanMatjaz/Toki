#include "resource_utils.h"

namespace Toki {

bool ResourceUtils::pathExists(std::filesystem::path path) {
    return std::filesystem::exists(path);
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

}  // namespace Toki
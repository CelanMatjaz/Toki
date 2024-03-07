#pragma once

#include <filesystem>

namespace Toki {

class Directory {
public:
    static std::filesystem::path workingDirectory();

    static std::filesystem::path cacheDirectory();

    // Assets
    static std::filesystem::path assetsDirectory();
    static std::filesystem::path shadersDirectory();
};

}  // namespace Toki

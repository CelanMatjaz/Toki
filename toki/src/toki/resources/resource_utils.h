#pragma once

#include <filesystem>

namespace Toki {

namespace ResourceUtils {

bool pathExists(std::filesystem::path path);
bool directoryExists(std::filesystem::path path);
bool fileExists(std::filesystem::path path);

}  // namespace ResourceUtils

}  // namespace Toki

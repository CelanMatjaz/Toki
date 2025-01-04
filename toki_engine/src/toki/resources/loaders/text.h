#pragma once

#include <filesystem>
#include <string>

namespace toki {

std::string read_text_file(std::filesystem::path path);

}  // namespace toki

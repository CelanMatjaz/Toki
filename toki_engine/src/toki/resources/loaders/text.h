#pragma once

#include <filesystem>
#include <string>

namespace toki {

namespace loaders {

std::string read_text_file(std::filesystem::path path);

}

}  // namespace toki

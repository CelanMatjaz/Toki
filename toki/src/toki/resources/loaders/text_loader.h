#pragma once

#include <expected>
#include <filesystem>
#include <string>

#include "toki/core/errors.h"

namespace Toki {

class TextLoader {
public:
    static std::expected<std::string, Error> readTextFile(std::filesystem::path filePath);
};

}  // namespace Toki

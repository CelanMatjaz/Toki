#pragma once

#include <expected>
#include <filesystem>
#include <string>

#include "toki/core/errors.h"

namespace Toki {

class BinaryLoader {
public:
    static std::expected<std::vector<uint8_t>, Error> readBinaryFile(std::filesystem::path filePath);
};

}
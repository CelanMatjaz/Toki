#pragma once

#include <expected>
#include <filesystem>

#include "toki/resources/configs.h"
#include "toki/resources/resource_types.h"

namespace Toki {

class ConfigLoader {
public:
    static std::expected<AttachmentsConfig, Error> loadAttachmentsConfig(const std::filesystem::path& path);
    static std::expected<ShaderConfig, Error> loadShaderConfig(const std::filesystem::path& path);
};

}  // namespace Toki

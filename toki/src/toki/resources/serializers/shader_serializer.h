#pragma once

#include <vector>

#include "toki/core/core.h"
#include "toki/core/errors.h"
#include "toki/resources/resource.h"

namespace Toki {

using ShaderSource = std::string;
using ShaderBinary = std::vector<uint32_t>;

class ShaderSerializer {
public:
    static Error serialize(std::filesystem::path fileName, std::vector<uint32_t>& shaderCode);
    static std::expected<std::vector<uint32_t>, Error> deserialize(const std::filesystem::path& path);

    static Ref<ShaderSource> loadShaderSource(std::filesystem::path path);
    static Ref<ShaderBinary> loadShaderBinary(std::filesystem::path path);
};

}  // namespace Toki

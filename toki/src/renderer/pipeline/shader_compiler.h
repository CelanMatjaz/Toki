#pragma once

#include <expected>
#include <string>
#include <vector>

#include "renderer/pipeline/vulkan_pipeline.h"
#include "toki/core/errors.h"

namespace Toki {

class ShaderCompiler {
public:
    static std::vector<uint32_t> compileShader(std::string name, const ShaderSource& source, bool cacheResult = true);

    static void serializeSPIRV(std::string name, const std::vector<uint32_t>& spirv);
    static std::expected<std::vector<uint32_t>, Error> deserializeSPIRV(std::string name);

private:
};

}  // namespace Toki

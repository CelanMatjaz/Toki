#pragma once

#include "filesystem"
#include "renderer/renderer_types.h"
#include "string"
#include "unordered_map"
#include "vector"

namespace Toki {

class ShaderLoader {
public:
    static std::unordered_map<ShaderStage, std::vector<uint32_t>> loadShader(std::filesystem::path path);

private:
    static std::string readShaderFile(std::filesystem::path path);
    static std::unordered_map<ShaderStage, std::string> parseShaderSource(std::string shaderSource);
};

}  // namespace Toki

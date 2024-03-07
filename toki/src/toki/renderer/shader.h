#pragma once

#include <filesystem>
#include <unordered_map>
#include <vector>

#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

struct ShaderConfig {
    std::unordered_map<ShaderStage, std::filesystem::path> shaderStagePaths;
    VertexLayoutDescriptions layoutDescriptions;
    std::vector<Attachment> attachments;
};

class Shader {
public:
    static Ref<Shader> create(const ShaderConfig& config);

    Shader(const ShaderConfig& config);
    Shader(const Shader& other) = delete;
    Shader(Shader&& other) = delete;
    Shader& operator=(const Shader& other) = delete;
    Shader& operator=(const Shader&& other) = delete;
    virtual ~Shader() = default;

protected:
    ShaderConfig m_config;

private:
    Shader() = default;
};

}  // namespace Toki

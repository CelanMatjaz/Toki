#pragma once

#include <filesystem>
#include <unordered_map>
#include <variant>
#include <vector>

#include "toki/core/core.h"
#include "toki/renderer/buffer.h"
#include "toki/renderer/renderer_types.h"
#include "toki/renderer/sampler.h"
#include "toki/renderer/texture.h"

namespace Toki {

struct GraphicsShaderOptions {
    PrimitiveTopology primitiveTopology : 4 = PrimitiveTopology::NotSpecified;
    CullMode cullMode : 4 = CullMode::NotSpecified;
    PolygonMode polygonMode : 2 = PolygonMode::NotSpecified;
    FrontFace frontFace : 2 = FrontFace::NotSpecified;
    bool primitiveRestart : 1 = false;
    DepthTest depthTest;
    StencilTest stencilTest;
    VertexLayoutDescriptions layoutDescriptions;
    std::vector<Attachment> attachments;
};

struct ComputeShaderOptions {};

using ShaderOptions = std::variant<GraphicsShaderOptions, ComputeShaderOptions>;
using ShaderStages = std::unordered_map<ShaderStage, std::variant<std::string, std::filesystem::path>>;

struct ShaderConfig {
    ShaderStages shaderStages;
    ShaderOptions options;
};

using UniformType = std::variant<Ref<UniformBuffer>, Ref<Texture>, Ref<Sampler>>;

struct Uniform {
    Uniform(UniformType u, uint8_t setIndex = 0, uint8_t binding = 0, uint8_t arrayElementIndex = 0) :
        uniform(u),
        setIndex(setIndex),
        binding(binding),
        arrayElementIndex(arrayElementIndex){};

    UniformType uniform;
    uint8_t setIndex = 0, binding = 0, arrayElementIndex = 0;
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

    virtual void setUniforms(std::vector<Uniform> uniforms) = 0;

protected:
    ShaderConfig m_config;

private:
    Shader() = default;
};

}  // namespace Toki

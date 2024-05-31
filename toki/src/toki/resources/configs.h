#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "toki/core/containers.h"
#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"
#include "toki/resources/resource.h"

namespace Toki {

struct AttachmentsConfig {
    std::vector<Attachment> attachments;
};

enum class AttributeType : uint8_t {
    vec1,
    vec2,
    vec3,
    vec4,
};

struct Attribute {
    uint8_t location;
    AttributeType type;
    uint8_t binding;

    uint8_t getTypeSize() {
        switch (type) {
            case AttributeType::vec1:
                return sizeof(float) * 1;
            case AttributeType::vec2:
                return sizeof(float) * 2;
            case AttributeType::vec3:
                return sizeof(float) * 3;
            case AttributeType::vec4:
                return sizeof(float) * 4;
            default:
                std::unreachable();
        }
    }

    Attribute(std::tuple<uint8_t, AttributeType, uint8_t> t) : location(std::get<0>(t)), type(std::get<1>(t)), binding(std::get<2>(t)) {}
};

struct Binding {
    uint8_t binding;
    VertexInputRate type;  // TODO: rename type to binding type
    uint8_t stride = 0;

    Binding(std::tuple<uint8_t, VertexInputRate, uint8_t> t) : binding(std::get<0>(t)), type(std::get<1>(t)), stride(std::get<2>(t)) {}
};

struct GraphicsShaderOptions {
    PrimitiveTopology primitiveTopology : 4 = PrimitiveTopology::NotSpecified;
    CullMode cullMode : 4 = CullMode::NotSpecified;
    PolygonMode polygonMode : 2 = PolygonMode::NotSpecified;
    FrontFace frontFace : 2 = FrontFace::NotSpecified;
    bool primitiveRestart : 1 = false;
    DepthTest depthTest;
    StencilTest stencilTest;
    std::vector<Attachment> attachments;
};

struct ComputeShaderOptions {};

using ShaderOptions = GraphicsShaderOptions;
using ShaderStages = std::unordered_map<ShaderStage, Ref<Resource>>;

struct ShaderStageType {
    ShaderStage stage;
    std::filesystem::path shaderSourcePath;
    std::filesystem::path shaderBinaryPath;

    ShaderStageType(std::tuple<ShaderStage, std::filesystem::path, std::filesystem::path> t) :
        stage(std::get<0>(t)),
        shaderSourcePath(std::get<1>(t)),
        shaderBinaryPath(std::get<2>(t)) {}
};

struct ShaderConfig {
    ShaderOptions options;
    std::vector<Attachment> attachments;
    std::vector<ShaderStageType> stages;
    std::vector<Attribute> attributes;
    std::vector<Binding> bindings;
};

struct FramebufferConfig {
    std::vector<Attachment> attachments;
    Extent3D extent;
};

enum class BufferType {
    None,
    VertexBuffer,
    IndexBuffer,
    UniformBuffer
};

struct BufferConfig {
    uint32_t size;
    BufferType type;
};

struct SamplerConfig {};

struct TextureConfig {
    ColorFormat format;
    Extent3D size;
};

struct VertexBufferBinding {
    Handle handle;
    uint32_t binding;
    uint32_t offset;
};

}  // namespace Toki

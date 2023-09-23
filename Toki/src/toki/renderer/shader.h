#pragma once

#include "core/core.h"
#include "filesystem"
#include "utility"
#include "vector"
#include "framebuffer.h"

namespace Toki {

    enum class ShaderStage {
        Vertex,
        Fragment,
        SHADER_STAGE_MAX_ENUM
    };

    enum class ShaderType {
        None, Graphics
    };

    enum class VertexInputRate {
        Vertex, Instance
    };

    enum class VertexFormat {
        Float1, Float2, Float3, Float4,
        Int1, Int2, Int3, Int4
    };

    struct VertexAttributeDescription {
        uint32_t location;
        uint32_t binding;
        VertexFormat format;
        uint32_t offset;
    };

    struct VertexBindingDescription {
        uint32_t binding;
        uint32_t stride;
        VertexInputRate inputRate;
    };

    enum class PrimitiveTopology {
        PointList, LineList, LineStrip, TriangleList, TriangleStrip, TriangleFan
    };

    enum class FrontFace {
        CounterClockwise, Clockwise
    };

    enum class CullMode {
        None, Front, Back, FrontAndBack
    };

    struct PipelineProperties {
        bool wireframe = false;
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
        FrontFace frontFace = FrontFace::CounterClockwise;
        CullMode cullMode = CullMode::Back;
        bool enableDepthTest = true;
        bool enableStencilTest = false;
    };

    struct ShaderConfig {
        ShaderType type = ShaderType::None;
        std::filesystem::path path;
        std::vector<VertexAttributeDescription> attributeDescriptions;
        std::vector<VertexBindingDescription> bindingDescriptions;
        Ref<Framebuffer> framebuffer;
        PipelineProperties properties{};
    };

    class Shader {
    public:
        static Ref<Shader> create(const ShaderConfig& config);
        Shader(const ShaderConfig& config);
        virtual ~Shader() = default;
        virtual void bind() = 0;
        virtual void reload() = 0;

    protected:
        ShaderConfig config;
    };

}
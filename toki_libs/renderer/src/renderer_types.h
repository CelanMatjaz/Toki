#pragma once

#include <toki/core.h>

namespace toki {

enum class ColorFormat : u16 {
    None,
    R8,
    RGBA8,
    Depth,
    Stencil,
    DepthStencil,

    COLOR_FORMAT_COUNT = DepthStencil
};

enum class ShaderType : u8 {
    GRAPHICS
};

enum class ShaderStage : u8 {
    Vertex,
    Fragment,
    SHADER_STAGE_COUNT
};

enum class VertexInputRate : u8 {
    Vertex,
    Instance,
};

enum class VertexFormat : u8 {
    FLOAT1,
    FLOAT2,
    FLOAT3,
    FLOAT4,
};

struct VertexAttributeDescription {
    uint32_t location;
    uint32_t binding;
    VertexFormat format;
    uint32_t offset;
};

struct VertexBindingDescription {
    u32 binding;
    u32 stride;
    VertexInputRate inputRate;
};

enum class BufferType : u8 {
    None,
    Vertex,
    Index,
    Uniform,
    BUFFER_TYPE_COUNT = Uniform,
};

enum class CompareOp : u8 {
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
};

enum class PrimitiveTopology : u8 {
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    LineListWithAdjacency,
    LineStripWithAdjacency,
    TriangleListWithAdjacency,
    TriangleStripWithAdjacency,
    PatchList,
};

enum class CullMode : u8 {
    None,
    Front,
    Back,
    Both
};

enum class PolygonMode : u8 {
    Fill,
    Line,
    Point,
};

enum class FrontFace : u8 {
    CounterClockwise,
    Clockwise,
};

struct Framebuffer {};

struct Buffer {};

struct Texture {};

struct Shader {};

struct FramebufferConfig {
    ColorFormat color_format;
    u32 color_attachment_count;
    u32 image_width;
    u32 image_height;
    b8 has_depth_attachment : 1;
    b8 has_stencil_attachment : 1;
};

struct BufferConfig {
    BufferType type;
    u32 size;
};

struct TextureConfig {
    ColorFormat format;
    u32 width;
    u32 height;
};

struct ShaderConfig {
    std::string_view config_path;
};

}  // namespace toki

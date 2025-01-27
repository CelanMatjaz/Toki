#pragma once

#include "core/core.h"

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
    VERTEX,
    INSTANCE,
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

enum class BufferUsage : u8 {
    NONE,
    STATIC,
    DYNAMIC,
};

enum class CompareOp : u8 {
    NEVER,
    LESS,
    EQUAL,
    LESS_OR_EQUAL,
    GREATER,
    NOT_EQUAL,
    GREATER_OR_EQUAL,
    ALWAYS,
};

enum class PrimitiveTopology : u8 {
    POINT_LIST,
    LINE_LIST,
    LINE_STRIP,
    TRIANGLE_LIST,
    TRIANGLE_STRIP,
    TRIANGLE_FAN,
    LINE_LIST_WITH_ADJACENCY,
    LINE_STRIP_WITH_ADJACENCY,
    TRIANGLE_LIST_WITH_ADJACENCY,
    TRIANGLE_STRIP_WITH_ADJACENCY,
    PATCH_LIST,
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

struct BeginPassConfig {
    Rect2D renderArea{};
    Handle framebufferHandle;
    Vec4 clearValue;
};

}  // namespace toki

#pragma once

#include "math/types.h"
#include "math/vector.h"

namespace toki {

enum class ColorFormat : u8 {
    NONE,
    R8,
    RGBA8,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL,

    COLOR_FORMAT_COUNT
};

enum class RenderTargetLoadOp : u8 {
    LOAD,
    CLEAR,
    DONT_CARE,
};

enum class RenderTargetStoreOp : u8 {
    STORE,
    DONT_CARE,
};

struct RenderTarget {
    ColorFormat colorFormat = ColorFormat::NONE;
    RenderTargetLoadOp loadOp : 2 = RenderTargetLoadOp::LOAD;
    RenderTargetStoreOp storeOp : 2 = RenderTargetStoreOp::STORE;
    bool presentable : 2 = false;
};

enum class ShaderStage : u8 {
    VERTEX,
    FRAGMENT
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
    uint32_t binding;
    uint32_t stride;
    VertexInputRate inputRate;
};

enum class BufferType : u8 {
    NONE,
    VERTEX,
    INDEX
};

enum class BufferUsage : u8 {
    NONE,
    STATIC,
    DYNAMIC,
};

struct BeginPassConfig {
    Rect2D renderArea{};
    Handle framebufferHandle{};
    Vector4<f32> clearValue;
};

}  // namespace toki

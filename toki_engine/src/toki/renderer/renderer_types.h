#pragma once

#include "core/core.h"

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
    b8 presentable : 2 = false;
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
    u32 binding;
    u32 stride;
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
    Vec4 clearValue;
    glm::mat4 viewProjectionMatrix;
};

}  // namespace toki

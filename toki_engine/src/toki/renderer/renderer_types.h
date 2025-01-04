#pragma once

#include "core/core.h"
namespace toki {

enum class ColorFormat {
    R8,
    RGBA8,
    DepthStencil,
};

enum class AttachmentLoadOp : u8 {
    Load,
    Clear,
    DontCare,
};

enum class AttachmentStoreOp : u8 {
    Store,
    DontCare,
};

enum class AttachmentType : u8 {
    Color,
    Depth,
    Stencil,
    DepthStencil
};

struct Attachment {
    ColorFormat colorFormat;
    AttachmentLoadOp loadOp : 4 = AttachmentLoadOp::Load;
    AttachmentStoreOp storeOp : 4 = AttachmentStoreOp::DontCare;
    AttachmentType typeBits : 4 = AttachmentType::Color;
    bool presentable : 4 = false;
};

enum class ShaderStage {
    Vertex,
    Fragment
};

enum class VertexInputRate {
    Vertex,
    Instance,
};

enum class VertexFormat {
    Float1,
    Float2,
    Float3,
    Float4,
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

}  // namespace toki

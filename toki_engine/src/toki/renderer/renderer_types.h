#pragma once

#include "core/core.h"

namespace toki {

enum class ColorFormat : u8 {
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
    AttachmentLoadOp loadOp : 2 = AttachmentLoadOp::Load;
    AttachmentStoreOp storeOp : 2 = AttachmentStoreOp::DontCare;
    AttachmentType typeBits : 2 = AttachmentType::Color;
    bool presentable : 2 = false;
};

enum class ShaderStage : u8 {
    Vertex,
    Fragment
};

enum class VertexInputRate : u8 {
    Vertex,
    Instance,
};

enum class VertexFormat : u8 {
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

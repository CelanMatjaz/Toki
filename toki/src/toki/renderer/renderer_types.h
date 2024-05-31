#pragma once

#include <cstdint>
#include "toki/core/id.h"

namespace Toki {

using Handle = Id;

enum class ColorFormat : uint8_t {
    R8,
    RG8,
    RGBA8,
    Depth,
    Stencil,
    DepthStencil,
};

enum AttachmentTypeBits : uint8_t {
    ATTACHMENT_TYPE_COLOR = 0x1,
    ATTACHMENT_TYPE_DEPTH = 0x2,
    ATTACHMENT_TYPE_STENCIL = 0x4,
};

enum class Samples : uint8_t {
    Count1,
    Count2,
    Count4,
    Count8,
    Count16,
    Count32,
    Count64,
};

enum class AttachmentLoadOp : uint8_t {
    Load,
    Clear,
    DontCare,
};

enum class AttachmentStoreOp : uint8_t {
    Store,
    DontCare,
};

struct Attachment {
    ColorFormat colorFormat;
    AttachmentLoadOp loadOp : 4 = AttachmentLoadOp::Load;
    AttachmentStoreOp storeOp : 4 = AttachmentStoreOp::DontCare;
    AttachmentTypeBits typeBits : 4 = AttachmentTypeBits::ATTACHMENT_TYPE_COLOR;
    bool presentable : 4 = false;
};

enum class IndexSize : uint8_t {
    INDEX_SIZE_32,
    INDEX_SIZE_16,
};

enum class ShaderStage {
    Vertex,
    Fragment
};

enum VertexInputRate {
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

enum class PrimitiveTopology : uint8_t {
    NotSpecified,
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

enum class PolygonMode : uint8_t {
    NotSpecified,
    Fill,
    Line,
    Point,
};

enum class CullMode : uint8_t {
    NotSpecified,
    None,
    Front,
    Back,
    FrontAndBack,
};

enum class FrontFace : uint8_t {
    NotSpecified,
    CounterClockwise,
    Clockwise,
};

enum class CompareOp : uint8_t {
    NotSpecified,
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
};

struct DepthTest {
    bool enable : 1 = false;
    bool write : 1 = false;
    CompareOp compareOp : 4;
};

struct StencilTest {
    bool enable : 1 = false;
};

}  // namespace Toki

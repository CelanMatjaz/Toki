#pragma once

#include <cstdint>

namespace Toki {

enum class ColorFormat : uint8_t {
    // COLOR_FORMAT_R,     // Maps to VK_FORMAT_R8_SRGB
    // COLOR_FORMAT_RG,    // Maps to VK_FORMAT_R8G8_SRGB
    // COLOR_FORMAT_RGB,   // Maps to VK_FORMAT_R8G8B8_SRGB
    COLOR_FORMAT_RGBA,  // Maps to VK_FORMAT_R8G8B8A8_SRGB
    COLOR_FORMAT_DEPTH,
    COLOR_FORMAT_STENCIL,
    COLOR_FORMAT_DEPTH_STENCIL,
};

enum AttachmentTypeBits : uint8_t {
    ATTACHMENT_TYPE_COLOR = 0x1,
    ATTACHMENT_TYPE_DEPTH = 0x2,
    ATTACHMENT_TYPE_STENCIL = 0x4,
};

enum class SampleCount : uint8_t {
    SAMPLE_COUNT_1,
    SAMPLE_COUNT_2,
    SAMPLE_COUNT_4,
    SAMPLE_COUNT_8,
    SAMPLE_COUNT_16,
    SAMPLE_COUNT_32,
    SAMPLE_COUNT_64,
};

enum class AttachmentLoadOp : uint8_t {
    ATTACHMENT_LOAD_OP_DONT_CARE,
    ATTACHMENT_LOAD_OP_LOAD,
    ATTACHMENT_LOAD_OP_CLEAR,
};

enum class AttachmentStoreOp : uint8_t {
    ATTACHMENT_STORE_OP_STORE,
    ATTACHMENT_STORE_OP_DONT_CARE,
};

struct Attachment {
    ColorFormat colorFormat;
    AttachmentLoadOp loadOp : 4 = AttachmentLoadOp::ATTACHMENT_LOAD_OP_LOAD;
    AttachmentStoreOp storeOp : 4 = AttachmentStoreOp::ATTACHMENT_STORE_OP_DONT_CARE;
    AttachmentTypeBits typeBits : 4 = AttachmentTypeBits::ATTACHMENT_TYPE_COLOR;
    bool presentable : 4 = false;
};

enum class IndexSize : uint8_t {
    INDEX_SIZE_32,
    INDEX_SIZE_16,
};

enum class BufferType {
    BUFFER_TYPE_VERTEX,
    BUFFER_TYPE_INDEX,
    BUFFER_TYPE_UNIFORM
};

enum class ShaderStage {
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_FRAGMENT
};

enum VertexInputRate {
    VERTEX_INPUT_RATE_VERTEX,
    VERTEX_INPUT_RATE_INSTANCE,
};

enum class VertexFormat {
    VERTEX_FORMAT_FLOAT1,
    VERTEX_FORMAT_FLOAT2,
    VERTEX_FORMAT_FLOAT3,
    VERTEX_FORMAT_FLOAT4,
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

struct VertexLayoutDescriptions {
    std::vector<VertexAttributeDescription> attributeDescriptions;
    std::vector<VertexBindingDescription> bindingDescriptions;
};

}  // namespace Toki
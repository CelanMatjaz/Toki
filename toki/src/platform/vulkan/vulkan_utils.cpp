#include "vulkan_utils.h"

#include "core/assert.h"
#include "renderer/renderer_types.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    TK_ASSERT(false, "Failed to find suitable memory type");

    return -1;
}

VkFormat mapFormat(Format format) {
    switch (format) {
        case Format::R:
            return VK_FORMAT_R8_SRGB;
        case Format::RG:
            return VK_FORMAT_R8G8_SRGB;
        case Format::RGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case Format::RGBA:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case Format::Depth:
            return VK_FORMAT_D32_SFLOAT;
        case Format::DepthStencil:
            return VK_FORMAT_D24_UNORM_S8_UINT;

        case Format::None:
            TK_ASSERT(false, "Format::None provided in function mapFormat");
            return VK_FORMAT_MAX_ENUM;
    }

    return VK_FORMAT_MAX_ENUM;
}

VkAttachmentLoadOp mapAttachmentLoadOp(AttachmentLoadOp loadOp) {
    switch (loadOp) {
        case AttachmentLoadOp::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case AttachmentLoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
    }

    return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
}

VkAttachmentStoreOp mapAttachmentStoreOp(AttachmentStoreOp storeOp) {
    switch (storeOp) {
        case AttachmentStoreOp::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case AttachmentStoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
    }

    return VK_ATTACHMENT_STORE_OP_DONT_CARE;
}

VkSampleCountFlagBits mapSamples(SampleCount sampleCount) {
    switch (sampleCount) {
        case SampleCount::SampleCount1:
            return VK_SAMPLE_COUNT_1_BIT;
        case SampleCount::SampleCount2:
            return VK_SAMPLE_COUNT_2_BIT;
        case SampleCount::SampleCount4:
            return VK_SAMPLE_COUNT_4_BIT;
        case SampleCount::SampleCount8:
            return VK_SAMPLE_COUNT_8_BIT;
        case SampleCount::SampleCount16:
            return VK_SAMPLE_COUNT_16_BIT;
        case SampleCount::SampleCount32:
            return VK_SAMPLE_COUNT_32_BIT;
        case SampleCount::SampleCount64:
            return VK_SAMPLE_COUNT_64_BIT;
    }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkShaderStageFlagBits mapShaderStage(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::None:
            TK_ASSERT(false, "Format::None provided in function mapShaderStage");
            return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }

    return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}

VkPrimitiveTopology mapPrimitiveTopology(PrimitiveTopology topology) {
    switch (topology) {
        case PrimitiveTopology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PrimitiveTopology::LineListWithAdjecency:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::LineStripWithAdjecency:
            return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleListWithAdjecency:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleStripWithAdjecency:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
        case PrimitiveTopology::PatchList:
            return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    }

    return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkPolygonMode mapPolygonMode(PolygonMode polygonMode) {
    switch (polygonMode) {
        case PolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;
        case PolygonMode::Line:
            return VK_POLYGON_MODE_LINE;
        case PolygonMode::Point:
            return VK_POLYGON_MODE_POINT;
    }

    return VK_POLYGON_MODE_MAX_ENUM;
}

VkCullModeFlagBits mapCullMode(CullMode cullMode) {
    switch (cullMode) {
        case CullMode::None:
            return VK_CULL_MODE_NONE;
        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        case CullMode::FrontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
    }

    return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}

VkFrontFace mapFrontFace(FrontFace frontFace) {
    switch (frontFace) {
        case FrontFace::Clockwise:
            return VK_FRONT_FACE_CLOCKWISE;
        case FrontFace::CounterClockwise:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    }

    return VK_FRONT_FACE_MAX_ENUM;
}

}  // namespace Toki

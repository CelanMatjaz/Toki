#include "mapping_functions.h"

#include <utility>

#include "renderer/vulkan_types.h"
#include "toki/renderer/renderer_types.h"

namespace Toki {

VkFormat mapFormat(ColorFormat format) {
    switch (format) {
        case ColorFormat::R8:
            return VK_FORMAT_R8_SRGB;
        case ColorFormat::RG8:
            return VK_FORMAT_R8G8_SRGB;
        case ColorFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::Depth:
        case ColorFormat::Stencil:
        case ColorFormat::DepthStencil:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        default:
            std::unreachable();
    }
}

VkSampleCountFlagBits mapSamples(Samples samples) {
    switch (samples) {
        case Samples::Count1:
            return VK_SAMPLE_COUNT_1_BIT;
        case Samples::Count2:
            return VK_SAMPLE_COUNT_2_BIT;
        case Samples::Count4:
            return VK_SAMPLE_COUNT_4_BIT;
        case Samples::Count8:
            return VK_SAMPLE_COUNT_8_BIT;
        case Samples::Count16:
            return VK_SAMPLE_COUNT_16_BIT;
        case Samples::Count32:
            return VK_SAMPLE_COUNT_32_BIT;
        case Samples::Count64:
            return VK_SAMPLE_COUNT_64_BIT;
        default:
            std::unreachable();
    }
}

VkAttachmentLoadOp mapLoadOp(AttachmentLoadOp loadOp) {
    switch (loadOp) {
        case AttachmentLoadOp::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        case AttachmentLoadOp::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case AttachmentLoadOp::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        default:
            std::unreachable();
    }
}

VkAttachmentStoreOp mapStoreOp(AttachmentStoreOp storeOp) {
    switch (storeOp) {
        case AttachmentStoreOp::DontCare:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        case AttachmentStoreOp::Store:
            return VK_ATTACHMENT_STORE_OP_STORE;
        default:
            std::unreachable();
    }
}

VkFormat mapVertexFormat(VertexFormat format) {
    switch (format) {
        case VertexFormat::Float1:
            return VK_FORMAT_R32_SFLOAT;
        case VertexFormat::Float2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::Float3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::Float4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:
            std::unreachable();
    }
}

VkShaderStageFlagBits mapShaderStage(ShaderStage shaderStage) {
    switch (shaderStage) {
        case ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        default:
            std::unreachable();
    }
}

uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) {
    for (uint32_t i = 0; i < context.physicalDeviceData.memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && (context.physicalDeviceData.memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    TK_ASSERT(false, "Failed to find suitable memory type");

    return -1;
}

}  // namespace Toki

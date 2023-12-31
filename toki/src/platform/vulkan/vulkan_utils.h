#pragma once

#include "renderer/renderer_types.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties);

// Mapping functions
VkFormat mapFormat(Format format);

VkAttachmentLoadOp mapAttachmentLoadOp(AttachmentLoadOp loadOp);

VkAttachmentStoreOp mapAttachmentStoreOp(AttachmentStoreOp storeOp);

VkSampleCountFlagBits mapSamples(SampleCount sampleCount);

VkShaderStageFlagBits mapShaderStage(ShaderStage stage);

VkPrimitiveTopology mapPrimitiveTopology(PrimitiveTopology topology);

VkPolygonMode mapPolygonMode(PolygonMode polygonMode);

VkCullModeFlagBits mapCullMode(CullMode cullMode);

VkFrontFace mapFrontFace(FrontFace frontFace);

}  // namespace Toki

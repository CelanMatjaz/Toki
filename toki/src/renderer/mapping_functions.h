#pragma once

#include <vulkan/vulkan.h>

#include "toki/renderer/renderer_types.h"

namespace Toki {

VkFormat mapFormat(ColorFormat format);
VkSampleCountFlagBits mapSamples(Samples samples);
VkAttachmentLoadOp mapLoadOp(AttachmentLoadOp loadOp);
VkAttachmentStoreOp mapStoreOp(AttachmentStoreOp storeOp);
VkFormat mapVertexFormat(VertexFormat format);
VkShaderStageFlagBits mapShaderStage(ShaderStage shaderStage);

// TODO: move
uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);

}  // namespace Toki

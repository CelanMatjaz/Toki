#pragma once

#include <vulkan/vulkan.h>

#include "renderer/renderer_types.h"

namespace toki {

VkFormat map_format(ColorFormat format);
VkAttachmentLoadOp map_attachment_load_op(AttachmentLoadOp op);
VkAttachmentStoreOp map_attachment_load_op(AttachmentStoreOp op);

}  // namespace toki

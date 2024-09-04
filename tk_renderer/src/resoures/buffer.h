#pragma once

#include <toki/core.h>

#include "renderer_types.h"

namespace Toki {

struct BufferConfig {
    uint32_t size;
    VkBufferUsageFlags usage;
    BufferType type = BufferType::NONE;
};

[[nodiscard]] TkError create_buffer(VulkanState* state, const BufferConfig* buffer_config, RendererBuffer* renderer_buffer_out);
void destroy_buffer(VulkanState* state, RendererBuffer* renderer_buffer_out);

}  // namespace Toki

#pragma once

#include "renderer/vulkan/vulkan_context.h"

namespace toki {

namespace vulkan_commands {

void submit_single_use_command_buffer(RendererContext* ctx, std::function<void(VkCommandBuffer cmd)> fn);
}

}  // namespace toki

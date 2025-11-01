#pragma once

#include "toki/renderer/private/vulkan/vulkan_state.h"

namespace toki {

struct VulkanCommandsData {
	const VulkanState* state;
	VulkanCommandBuffer cmd;
};

}  // namespace toki

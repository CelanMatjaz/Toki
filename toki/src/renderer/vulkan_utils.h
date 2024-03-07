#pragma once

#include "renderer/vulkan_types.h"
#include "toki/core/window.h"

namespace Toki {

namespace VulkanUtils {

VkSurfaceKHR createSurface(Ref<VulkanContext> context, Ref<Window> window);
bool checkForMailboxPresentModeSupport(Ref<VulkanContext> context, VkSurfaceKHR surface);

VkImageAspectFlags getImageAspectFlags(VkFormat format);

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties);
}

}  // namespace Toki

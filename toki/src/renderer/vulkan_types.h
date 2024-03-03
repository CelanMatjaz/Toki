#pragma once

#include <vulkan/vulkan.h>

#if VK_NO_PROTOTYPES
PFN_vkCmdBeginRenderingKHR vkCmdBeginRenderingKHR{ VK_NULL_HANDLE };
PFN_vkCmdEndRenderingKHR vkCmdEndRenderingKHR{ VK_NULL_HANDLE };
#endif

#define MAX_FRAMES 3

namespace Toki {

struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t graphicsFamilyIndex = UINT32_MAX;
    uint32_t presentFamilyIndex = UINT32_MAX;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkAllocationCallbacks* allocationCallbacks = nullptr;
};

struct SwapchainConfig {};

struct FrameData {
    VkSemaphore presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderSemaphore = VK_NULL_HANDLE;
    VkFence renderFence = VK_NULL_HANDLE;

    uint32_t currentImageIndex = 0;
};

}  // namespace Toki
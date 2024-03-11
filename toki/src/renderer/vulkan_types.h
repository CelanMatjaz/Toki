#pragma once

#include <vulkan/vulkan.h>

#include <functional>

#include "toki/renderer/renderer_types.h"

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

    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

    VkAllocationCallbacks* allocationCallbacks = nullptr;

    std::function<void(std::function<void(VkCommandBuffer)>)> submitSingleUseCommands = {};
};

struct SwapchainConfig {
    bool useVSync : 1 = false;
};

struct FrameData {
    VkSemaphore presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderSemaphore = VK_NULL_HANDLE;
    VkFence renderFence = VK_NULL_HANDLE;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;  // Temp

    uint32_t currentImageIndex = 0;
    bool startedRecording = false;
};

}  // namespace Toki

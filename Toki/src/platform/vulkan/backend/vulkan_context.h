#pragma once

#include "vulkan/vulkan.h"
#include "platform/vulkan/backend/vulkan_swapchain.h"

namespace Toki {

    class VulkanContext {
        friend class VulkanRenderer;
    public:
        struct QueueFamilyIndexes {
            uint32_t graphicsQueueIndex;
            uint32_t presentQueueIndex;
        };

        static inline const uint32_t MAX_FRAMES = 3;

    public:
        VulkanContext();
        ~VulkanContext();

        void init();
        void shutdown();

    private:
        void createInstance();
        void createSurface();
        void createDevice();
        void initQueueFamilyIndexes();
        void initQueueFamilyProperties();
        void createDescriptorPool();
        void createCommandPool();
        void createFrames();

        std::vector<VkPhysicalDevice> enumeratePhysicalDevices();

    private:
        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;

        QueueFamilyIndexes queueFamilyIndexes = { UINT32_MAX, UINT32_MAX };
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        struct FrameData {
            VkSemaphore presentSemaphore = VK_NULL_HANDLE;
            VkSemaphore renderSemaphore = VK_NULL_HANDLE;
            VkFence renderFence = VK_NULL_HANDLE;
            VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        } frames[MAX_FRAMES];

        FrameData* getCurrentFrame() { return &frames[currentFrame]; }

        Ref<VulkanSwapchain> swapchain;

        uint8_t currentFrame = 0;
        uint32_t imageIndex = 0;
    };

}
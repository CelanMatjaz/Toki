#pragma once

#include "tkpch.h"
#include "vulkan_renderer.h"

namespace Toki {
    class VulkanDevice {
    public:
        struct QueueFamilyIndexes {
            uint32_t graphicsQueueIndex;
            uint32_t presentQueueIndex;
        };

        static VkDevice createDevice();
        static bool checkForValidationLayerSupport();
        static void initQueueFamilyIndexes();
        static void initQueueFamilyProperties();

        static std::vector<VkPhysicalDevice> enumeratePhysicalDevices();
        static QueueFamilyIndexes getQueueFamilyIndexes() { return queueFamilyIndexes; }
        static  std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() { return queueFamilyProperties; }

        // swapchain/image util functions
    public:
        static VkExtent2D getExtent(VkSurfaceCapabilitiesKHR capabilities);
        static uint32_t getImageCount(VkSurfaceCapabilitiesKHR capabilities, uint32_t minImageCount = 0); // always min image count
        static VkSurfaceTransformFlagBitsKHR getPreTransform(VkSurfaceCapabilitiesKHR capabilities);
        static VkCompositeAlphaFlagBitsKHR getCompositeAlpha(VkSurfaceCapabilitiesKHR capabilities);

        static VkImageTiling findTiling(VkFormat format);
        static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlagBits features);
        static VkSurfaceFormatKHR findSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);

    private:
        inline static QueueFamilyIndexes queueFamilyIndexes = { UINT32_MAX, UINT32_MAX };
        inline static std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    };
}
#pragma once

#include "tkpch.h"

namespace Toki {

    class VulkanDevice {

    public:
        VulkanDevice();
        ~VulkanDevice();

        struct QueueFamilyIndexes {
            uint32_t graphicsQueueIndex;
            uint32_t presentQueueIndex;
        };

        VkInstance getInstance() { return instance; }
        VkSurfaceKHR getSurface() { return surface; }
        VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
        VkDevice getDevice() { return deviceHandle; }

        std::vector<VkPhysicalDevice> enumeratePhysicalDevices();
        QueueFamilyIndexes getQueueFamilyIndexes() { return queueFamilyIndexes; }
        std::vector<VkQueueFamilyProperties> getQueueFamilyProperties() { return queueFamilyProperties; }

        // swapchain/image util functions
    public:
        static VkExtent2D getExtent();
        static uint32_t getImageCount(uint32_t maxImageCount = 0); // always min image count
        static VkSurfaceTransformFlagBitsKHR getPreTransform();
        static VkCompositeAlphaFlagBitsKHR getCompositeAlpha();

        static VkImageTiling findTiling(VkFormat format);
        static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlagBits features);
        static VkSurfaceFormatKHR findSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);

    private:
        void createInstance();
        void createSurface();
        void createDevice();
        void initQueueFamilyIndexes();
        void initQueueFamilyProperties();

        bool checkForValidationLayerSupport();

        VkInstance instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice;
        VkDevice deviceHandle;

        QueueFamilyIndexes queueFamilyIndexes = { UINT32_MAX, UINT32_MAX };
        std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    };
}
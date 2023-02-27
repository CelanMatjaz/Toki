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

        static vk::Device createDevice();
        static bool checkForValidationLayerSupport();
        static void initQueueFamilyIndexes();

        static QueueFamilyIndexes getQueueFamilyIndexes() { return queueFamilyIndexes; }

        // swapchain/image util functions
    public:
        static vk::Extent2D getExtent(vk::SurfaceCapabilitiesKHR capabilities);
        static uint32_t getImageCount(vk::SurfaceCapabilitiesKHR capabilities, uint32_t minImageCount = 0); // always min image count
        static vk::SurfaceTransformFlagBitsKHR getPreTransform(vk::SurfaceCapabilitiesKHR capabilities);
        static vk::CompositeAlphaFlagBitsKHR getCompositeAlpha(vk::SurfaceCapabilitiesKHR capabilities);

        static vk::ImageTiling findTiling(vk::Format format);
        static vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlagBits features);
        static vk::SurfaceFormatKHR findSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
        static uint32_t findMemoryType(uint32_t typeBits);

    private:
        inline static QueueFamilyIndexes queueFamilyIndexes = { UINT32_MAX, UINT32_MAX };
    };
}
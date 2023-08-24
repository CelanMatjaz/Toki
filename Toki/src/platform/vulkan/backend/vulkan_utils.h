#pragma once

#include "vulkan/vulkan.h"
#include "vector"
#include "renderer/framebuffer.h"
#include "renderer/shader.h"

namespace Toki {

    class VulkanUtils {
    public:
        static bool checkForValidationLayerSupport();

        static VkExtent2D getExtent();
        static uint32_t getImageCount(uint32_t maxImageCount = 0); // 0 means always get min image count
        static VkSurfaceTransformFlagBitsKHR getPreTransform();
        static VkCompositeAlphaFlagBitsKHR getCompositeAlpha();

        static VkImageTiling findTiling(VkFormat format);
        static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlagBits features);
        static VkSurfaceFormatKHR findSurfaceFormat();
        static uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);
        static VkFormat findDepthFormat();

        static VkFormat mapFormat(Format format);
        static VkSampleCountFlagBits mapSamples(Samples samples);
        static VkAttachmentLoadOp mapLoadOp(AttachmentLoadOp loadOp);
        static VkAttachmentStoreOp mapStoreOp(AttachmentStoreOp storeOp);
        static VkFormat mapVertexFormat(VertexFormat format);

    public:
        inline static const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        inline static const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

    };

}
#include "tkpch.h"
#include "vulkan_image.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "toki/core/assert.h"

namespace Toki {

    VulkanImage::VulkanImage(const VulkanImageConfig& config) {
        VkImageAspectFlags aspectMask = 0;
        VkImageLayout imageLayout;

        format = config.format;

        if (config.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (config.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (config.format >= VK_FORMAT_D16_UNORM_S8_UINT) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        TK_ASSERT(aspectMask > 0, "Aspect mask is not valid!");

        VkDevice device = VulkanRenderer::device();

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = config.format;
        imageCreateInfo.extent = config.extent;
        imageCreateInfo.mipLevels = config.mips;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = config.tiling;
        imageCreateInfo.usage = config.usage | VK_IMAGE_USAGE_SAMPLED_BIT;

        vkCreateImage(device, &imageCreateInfo, nullptr, &this->image);

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, this->image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = VulkanUtils::findMemoryType(memoryRequirements.memoryTypeBits, config.memoryProperties);

        TK_ASSERT_VK_RESULT(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &this->memory), "Could not allocate Vulkan image memory");
        TK_ASSERT_VK_RESULT(vkBindImageMemory(device, this->image, this->memory, 0), "Could not bind Vulkan image memory");

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = format;
        imageViewCreateInfo.subresourceRange = {};
        imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

        TK_ASSERT_VK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &this->view), "Could not create Vulkan image view");
    }

    VulkanImage::VulkanImage(VkImage image, VkImageView imageView, VkFormat format)
        : image(image), view(imageView), format(format), isWrapped(true) {}

    VulkanImage::~VulkanImage() {
        VkDevice device = VulkanRenderer::device();

        vkDestroyImageView(device, view, nullptr);

        if (!isWrapped) {
            vkDestroyImage(device, image, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }
    }

}
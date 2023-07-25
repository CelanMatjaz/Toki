#include "tkpch.h"
#include "vulkan_image.h"

#include "toki/core/application.h"
#include "infos.h"

namespace Toki {

    void VulkanImage::cleanup() {
        VkDevice device = Application::getVulkanRenderer()->getDevice();

        vkDestroyImageView(device, imageView, nullptr);
        vkDestroyImage(device, image, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

    VulkanImage VulkanImage::create(const VulkanImageSpec& spec) {
        VulkanImage image;

        VkImageAspectFlags aspectMask = 0;
        VkImageLayout imageLayout;

        image.format = spec.format;
        image.extent = spec.extent;

        if (spec.usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (spec.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (spec.format >= VK_FORMAT_D16_UNORM_S8_UINT) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        TK_ASSERT(aspectMask > 0);

        image.image = createImage(spec.format, spec.extent, spec.usage);

        VkDevice device = Application::getVulkanRenderer()->getDevice();

        VkMemoryRequirements memoryRequirements;
        vkGetImageMemoryRequirements(device, image.image, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = VulkanDevice::findMemoryType(memoryRequirements.memoryTypeBits, spec.memoryProperties);

        TK_ASSERT_VK_RESULT(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &image.memory));
        TK_ASSERT_VK_RESULT(vkBindImageMemory(device, image.image, image.memory, 0));

        image.imageView = createImageView(spec.format, aspectMask, image.image);

        return image;
    }

    VkImage VulkanImage::createImage(VkFormat format, VkExtent3D extent, VkImageUsageFlags usage) {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format = format;
        imageCreateInfo.extent = extent;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

        VkDevice device = Application::getVulkanRenderer()->getDevice();

        VkImage image;
        TK_ASSERT_VK_RESULT(vkCreateImage(device, &imageCreateInfo, nullptr, &image));

        return image;
    }

    VkImageView VulkanImage::createImageView(VkFormat format, VkImageAspectFlags aspectMask, VkImage image) {
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

        VkDevice device = Application::getVulkanRenderer()->getDevice();

        VkImageView imageView;
        TK_ASSERT_VK_RESULT(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView));

        return imageView;
    }

}
#include "tkpch.h"
#include "vulkan_image.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/vulkan/backend/vulkan_utils.h"
#include "platform/vulkan/vulkan_buffer.h"
#include "toki/core/assert.h"

namespace Toki {

    VulkanImage::VulkanImage(const VulkanImageConfig& config) {
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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

        extent = config.extent;

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

    void VulkanImage::setData(uint32_t size, void* data) {
        BufferConfig bufferConfig{};
        bufferConfig.size = size;
        VulkanBuffer stagingBuffer(bufferConfig, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.setData(size, data);

        this->transitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToTexture(&stagingBuffer, this);
        this->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    void VulkanImage::transitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
        auto commandBuffer = VulkanRenderer::beginSingleUseCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VulkanRenderer::endSingleUseCommands(commandBuffer);
    }

    void VulkanImage::copyBufferToTexture(VulkanBuffer* buffer, VulkanImage* texture) {
        VkDevice device = VulkanRenderer::device();
        VkCommandBuffer commandBuffer = VulkanRenderer::beginSingleUseCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { texture->getWidth(), texture->getHeight(), 1 };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer->getBuffer(),
            texture->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        VulkanRenderer::endSingleUseCommands(commandBuffer);
    }

    VkSampler VulkanImage::createSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(VulkanRenderer::physicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkSampler sampler;
        TK_ASSERT_VK_RESULT(vkCreateSampler(VulkanRenderer::device(), &samplerInfo, nullptr, &sampler), "");

        return sampler;
    }


}
#pragma once

#include "tkpch.h"

namespace Toki {

    namespace Infos {

        namespace Pipeline {

            VkPipelineShaderStageCreateInfo shaderStageCreateInfo(const VkShaderStageFlagBits& stage, const VkShaderModule& shaderModule, const char* name = "main");
            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& inputBindingDescriptions = {}, const std::vector<VkVertexInputAttributeDescription>& inputAttributeDescriptions = {});
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo(const VkPrimitiveTopology& topology, const VkBool32& primitiveRestartEnable = VK_FALSE);
            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(const VkPolygonMode& polygonMode, const VkFrontFace& frontFace, const VkCullModeFlags& cullMode);
            VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo(const VkBool32& depthTest, const VkBool32& depthWrite, const VkCompareOp& compareOp);
            VkPipelineColorBlendAttachmentState colorBlendAttachmentState(const VkBool32& blendEnable, const VkColorComponentFlags& writeMask);
            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& attachments = {});
            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo(const std::vector<VkDynamicState>& states = {});

        }

        namespace RenderPass {

            VkAttachmentDescription colorAttachmentDescription(const VkFormat& format);
            VkAttachmentDescription depthAttachmentDescription(const VkFormat& format);
            VkSubpassDescription subpassDescription(const std::vector<VkAttachmentReference>& colorAttachmentReferences, const VkAttachmentReference& depthReference);
            VkSubpassDependency colorSubpassDependency();
            VkSubpassDependency depthSubpassDependency();
            VkRenderPassCreateInfo renderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDependency>& dependencies, const std::vector<VkSubpassDescription>& subpasses);
            VkRenderPassBeginInfo renderPassBeginInfo(const VkRenderPass& renderPass, const VkExtent2D& extent, const VkFramebuffer& framebuffer, const std::vector<VkClearValue>& clearValues);

        }

        namespace Renderer {

            VkFenceCreateInfo fenceCreateInfo(const VkFenceCreateFlags& flags = VK_FENCE_CREATE_SIGNALED_BIT);
            VkSemaphoreCreateInfo semaphoreCreateInfo();
            VkCommandPoolCreateInfo commandPoolCreateInfo(VkCommandPoolCreateFlags flags, const uint32_t& queueFamilyIndex);
            VkSwapchainCreateInfoKHR swapchainCreateInfo();
            VkFramebufferCreateInfo framebufferCreateInfo(const VkRenderPass& renderPass, const uint32_t& width, const uint32_t& height, const uint32_t& layers = 1);
            VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
            VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlagBits flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
            VkSubmitInfo submitInfo(const std::vector<VkCommandBuffer>& commandBuffers, const VkPipelineStageFlags waitStages);
             VkSubmitInfo submitInfo(VkCommandBuffer commandBuffer);
            VkPresentInfoKHR presentInfo(const std::vector<VkSwapchainKHR>& swapchains, const std::vector<uint32_t>& imageIndices);

        }

        namespace Container {

            VkImageCreateInfo imageCreateInfo(const VkFormat& format, const VkImageTiling& tiling, const VkExtent3D& extent, VkImageUsageFlagBits usage, VkImageType imageType = VK_IMAGE_TYPE_2D);
            VkImageViewCreateInfo imageViewCreateInfo(const VkImage& image, const VkFormat& format, const VkImageViewType& viewType = VK_IMAGE_VIEW_TYPE_2D, const VkImageViewCreateFlags& flags = 0);
            VkMemoryAllocateInfo memoryAllocateInfo(const VkDeviceSize& size, const uint32_t& memoryTypeIndex);

        }

    }

}
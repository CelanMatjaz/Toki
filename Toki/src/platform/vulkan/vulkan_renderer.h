#pragma once

#include "platform/vulkan/backend/vulkan_context.h"
#include "core/core.h"
#include "renderer/renderer.h"

namespace Toki {

    class VulkanRenderer : public Renderer {
    public:
        VulkanRenderer() = default;
        ~VulkanRenderer() = default;

        virtual void init() override;
        virtual void shutdown() override;

        virtual void beginFrame() override;
        virtual void endFrame() override;

        static VkDevice device() { return context->device; }
        static VkPhysicalDevice physicalDevice() { return context->physicalDevice; }
        static VkSurfaceKHR surface() { return context->surface; }
        static VulkanContext::QueueFamilyIndexes queueFamilyIndexes() { return context->queueFamilyIndexes; }
        static auto swapchain() { return context->swapchain; }
        static uint32_t currentFrameIndex() { return context->currentFrame; }
        static VkDescriptorPool descriptorPool() { return context->descriptorPool; }

        static VkCommandBuffer commandBuffer() { return context->getCurrentFrame()->commandBuffer; }

    private:
        static Ref<VulkanContext> context;
    };

}
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

        virtual void resize(uint32_t width, uint32_t height, uint32_t layers = 1) override;

        static VkInstance instance() { return context->instance; }
        static VkDevice device() { return context->device; }
        static VkPhysicalDevice physicalDevice() { return context->physicalDevice; }
        static VkSurfaceKHR surface() { return context->surface; }
        static VulkanContext::QueueFamilyIndexes queueFamilyIndexes() { return context->queueFamilyIndexes; }
        static auto swapchain() { return context->swapchain; }
        static uint32_t currentFrameIndex() { return context->currentFrame; }
        static VkDescriptorPool descriptorPool() { return context->descriptorPool; }
        static VkQueue graphicsQueue() { return context->graphicsQueue; }

        static VkSampler sampler() { return context->sampler; }
        static Ref<VulkanImage> defaultTexture() { return context->noTexture; }
        static Ref<VulkanUniformBuffer> defaultUniform() { return context->noBuffer; }

        static Ref<VulkanContext> getContext() { return context; }

        static VkCommandBuffer commandBuffer() { return context->getCurrentFrame()->commandBuffer; }

        static VkCommandBuffer beginSingleUseCommands();
        static void endSingleUseCommands(VkCommandBuffer commandBuffer);

    private:
        static Ref<VulkanContext> context;
    };

}
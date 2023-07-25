#pragma once

#include "tkpch.h"
#include "toki/renderer/renderer_api.h"
#include "vulkan_device.h"
#include "vulkan_swapchain.h"

namespace Toki {
    class VulkanDevice;

    class VulkanRenderer : public RendererAPI {
    public:
        VulkanRenderer();
        ~VulkanRenderer();

        void init() override;
        void shutdown() override;

        void beginFrame() override;
        void endFrame() override;

        void onResize() override;

        VkInstance getInstance() { return device->getInstance(); }
        VkSurfaceKHR getSurface() { return device->getSurface(); }
        VkPhysicalDevice getPhysicalDevice() { return device->getPhysicalDevice(); }
        VkDevice getDevice() { return device->getDevice(); }

        VulkanSwapchain* getSwapchain() { return swapchain; }

        VkCommandPool getCommandPool() { return commandPool; }
        VkCommandBuffer getCommandBuffer() { return getCurrentFrame()->commandBuffer; }
        VkQueue getGraphicsQueue() { return graphicsQueue; }
        auto getQueueFamilyIndexes() { return device->getQueueFamilyIndexes(); }

        VkCommandBuffer startCommandBuffer();
        void endCommandBuffer(VkCommandBuffer commandBuffer);

        void recreateSwapchain(VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR) {
            shouldRecreateSwapchain = true;
            newPresentMode = presentMode;
        }

        static constexpr uint32_t MAX_FRAMES = 3;

    private:
        VulkanDevice* device;
        VulkanSwapchain* swapchain;

        VkCommandPool commandPool;
        VkCommandBuffer currentCommandBuffer;

        VkQueue graphicsQueue;
        VkQueue presentQueue;

        std::vector<VkFramebuffer> frameBuffers;

        void createCommandPool();
        void createRenderPass();
        void createFrames();

        struct FrameData {
            VkSemaphore presentSemaphore, renderSemaphore;
            VkFence renderFence;
            VkCommandBuffer commandBuffer;

            void cleanup() {
                // VkDevice device = Application::getVulkanRenderer()->getDevice();
                // vkDestroyFence(device, renderFence, nullptr);
                // vkDestroySemaphore(device, presentSemaphore, nullptr);
                // vkDestroySemaphore(device, renderSemaphore, nullptr);
            }
        } frames[MAX_FRAMES];

        uint32_t currentFrame{ 0 };
        uint32_t imageIndex;
        bool isFrameStarted = false;
        bool shouldRecreateSwapchain = false;
        VkPresentModeKHR newPresentMode = VK_PRESENT_MODE_FIFO_KHR;
        FrameData* getCurrentFrame() { return &frames[currentFrame % MAX_FRAMES]; }

    };

}
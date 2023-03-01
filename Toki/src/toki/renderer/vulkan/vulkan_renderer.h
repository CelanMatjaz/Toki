#pragma once

#include "tkpch.h"
#include "vulkan/vulkan.hpp"
#include "vulkan_device.h"

namespace Toki {

    class VulkanRenderer {
    public:
        VulkanRenderer();
        ~VulkanRenderer();

        void init();
        void cleanup();

        void beginFrame();
        void endFrame();

        static vk::Instance getInstance() { return instance; }
        static vk::SurfaceKHR getSurface() { return surface; }
        static vk::PhysicalDevice getPhysicalDevice() { return physicalDevice; }
        static vk::Device getDevice() { return device; }
        static vk::CommandPool getCommandPool() { return commandPool; }
        static vk::SwapchainKHR getSwapchain() { return swapchain; }
        static vk::RenderPass getRenderPass() { return renderPass; }
        static vk::CommandBuffer getCommandBuffer() { return currentCommandBuffer; }

        static vk::Extent2D getSwapchainExtent() { return swapchainExtent; }
        static vk::Format getSwapchainFormat() { return swapchainImageFormat; }
        static vk::Queue getGraphicsQueue() { return graphicsQueue; }

        uint32_t getImageCount() { return swapchainImages.size(); }

    private:
        inline static vk::Instance instance;
        inline static vk::SurfaceKHR surface;
        inline static vk::PhysicalDevice physicalDevice;
        inline static vk::Device device;
        inline static vk::CommandPool commandPool;
        inline static vk::SwapchainKHR swapchain;
        inline static vk::RenderPass renderPass;
        inline static vk::CommandBuffer currentCommandBuffer;

        inline static vk::Extent2D swapchainExtent;
        inline static vk::Format swapchainImageFormat;
        inline static vk::Queue graphicsQueue;
        inline static vk::Queue presentQueue;

        void recreateSwapchain();
        void cleanupSwapchain();

        void createInstance();
        void createSurface();
        void createCommandPool();
        void createSwapchain();
        void createRenderPass();
        void createFrames();
        void createFrameBuffers();
        void createDepthBuffer();

        static vk::CommandBuffer startCommandBuffer();
        static void endCommandBuffer(vk::CommandBuffer commandBuffer);

        static constexpr uint32_t MAX_FRAMES = 3;

        struct FrameData {
            vk::Semaphore presentSemaphore, renderSemaphore;
            vk::Fence renderFence;
            vk::CommandBuffer commandBuffer;

            void cleanup() {
                device.destroyFence(renderFence);
                device.destroySemaphore(presentSemaphore, nullptr, {});
                device.destroySemaphore(renderSemaphore, nullptr, {});
            }
        } frames[MAX_FRAMES];

        uint32_t currentFrame{ 0 };
        uint32_t imageIndex;
        bool isFrameStarted = false;
        FrameData* getCurrentFrame() { return &frames[currentFrame % MAX_FRAMES]; }

        struct Image {
            vk::Image image;
            vk::ImageView imageView;
            vk::DeviceMemory memory;

            void cleanup() {
                device.destroyImage(image);
                device.destroyImageView(imageView);
                device.freeMemory(memory);
            }
        };

        std::vector<vk::Image> swapchainImages;
        std::vector<vk::ImageView> swapchainImageViews;
        std::vector<vk::Framebuffer> frameBuffers;

        Image depthBuffer;
        vk::Format depthFormat;
    };

}
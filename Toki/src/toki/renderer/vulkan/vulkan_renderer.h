#pragma once

#include "tkpch.h"
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

        static VkInstance getInstance() { return instance; }
        static VkSurfaceKHR getSurface() { return surface; }
        static VkPhysicalDevice getPhysicalDevice() {
            int a = 0;
            return physicalDevice;
        }
        static VkDevice getDevice() { return device; }
        static VkCommandPool getCommandPool() { return commandPool; }
        static VkSwapchainKHR getSwapchain() { return swapchain; }
        static VkRenderPass getRenderPass() { return renderPass; }
        static VkCommandBuffer getCommandBuffer() { return currentCommandBuffer; }

        static VkExtent2D getSwapchainExtent() { return swapchainExtent; }
        static VkFormat getSwapchainFormat() { return swapchainImageFormat; }
        static VkQueue getGraphicsQueue() { return graphicsQueue; }

        uint32_t getImageCount() { return swapchainImages.size(); }

    private:
        inline static VkInstance instance;
        inline static VkSurfaceKHR surface;
        inline static VkPhysicalDevice physicalDevice;
        inline static VkDevice device;
        inline static VkCommandPool commandPool;
        inline static VkSwapchainKHR swapchain;
        inline static VkRenderPass renderPass;
        inline static VkCommandBuffer currentCommandBuffer;

        inline static VkExtent2D swapchainExtent;
        inline static VkFormat swapchainImageFormat;
        inline static VkQueue graphicsQueue;
        inline static VkQueue presentQueue;

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

        static VkCommandBuffer startCommandBuffer();
        static void endCommandBuffer(VkCommandBuffer commandBuffer);

        static constexpr uint32_t MAX_FRAMES = 3;

        struct FrameData {
            VkSemaphore presentSemaphore, renderSemaphore;
            VkFence renderFence;
            VkCommandBuffer commandBuffer;

            void cleanup() {
                vkDestroyFence(device, renderFence, nullptr);
                vkDestroySemaphore(device, presentSemaphore, nullptr);
                vkDestroySemaphore(device, renderSemaphore, nullptr);
            }
        } frames[MAX_FRAMES];

        uint32_t currentFrame{ 0 };
        uint32_t imageIndex;
        bool isFrameStarted = false;
        FrameData* getCurrentFrame() { return &frames[currentFrame % MAX_FRAMES]; }

        struct Image {
            VkImage image;
            VkImageView imageView;
            VkDeviceMemory memory;

            void cleanup() {
                vkDestroyImage(device, image, nullptr);
                vkDestroyImageView(device, imageView, nullptr);
                vkFreeMemory(device, memory, nullptr);
            }
        };

        std::vector<VkImage> swapchainImages;
        std::vector<VkImageView> swapchainImageViews;
        std::vector<VkFramebuffer> frameBuffers;

        Image depthBuffer;
        VkFormat depthFormat;
    };

}
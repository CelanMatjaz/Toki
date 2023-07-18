#pragma once

#include "tkpch.h"
#include "vulkan_render_pass.h"
#include "vulkan_frame_buffer.h"

namespace Toki {

    class VulkanSwapchain {
    public:
        VulkanSwapchain(VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR);
        ~VulkanSwapchain();

        void recreate(VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR);

    private:
        struct Image {
            VkImage image;
            VkImageView imageView;
            VkDeviceMemory memory;

            void cleanup(VkDevice device) {
                vkDestroyImage(device, image, nullptr);
                vkDestroyImageView(device, imageView, nullptr);
                vkFreeMemory(device, memory, nullptr);
            }
        };

    public:
        VkSwapchainKHR getSwapchainHandle() const { return swapchain; }
        VkExtent2D getExtent() const { return extent; }
        VkFormat getImageFormat() const { return imageFormat; }
        uint8_t getImageCount() const { return images.size(); }
        VkFormat getDepthFormat() const { return depthFormat; }
        const std::vector<VkImageView>& getImageViews() const { return imageViews; }
        const Image& getDepthBuffer() const { return depthBuffer; }
        VkRenderPass getRenderPass() const { return renderPass.getRenderPass(); }
        VulkanRenderPass getRenderPassHandle() const { return renderPass; }
        std::vector<VulkanFrameBuffer> getFrameBuffers() { return frameBuffers; }

    private:
        void create(VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR);
        void cleanup();
        void createDepthBuffer();

        void createRenderPass();
        void createFrameBuffers();

        VkSwapchainKHR swapchain;
        VkExtent2D extent;
        VkFormat imageFormat;

        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;

        Image depthBuffer;
        VkFormat depthFormat;

        VulkanRenderPass renderPass;
        std::vector<VulkanFrameBuffer> frameBuffers;
    };

}
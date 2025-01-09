#pragma once

#include "renderer/vulkan/vulkan_types.h"

namespace toki {

struct RendererContext;

class VulkanSwapchain {
public:
    struct Config {
        Ref<Window> window;
        VkCommandPool command_pool;
    };

    void create(Ref<RendererContext> ctx, const Config& config);
    void destroy(Ref<RendererContext> ctx);
    void recreate(Ref<RendererContext> ctx);

    b8 start_recording(Ref<RendererContext> ctx);
    void stop_recording(Ref<RendererContext> ctx);
    void end_frame(Ref<RendererContext> ctx);

    void prepare_current_frame_image();
    void transition_current_frame_image();

    VkSwapchainKHR get_handle() const;
    VkCommandBuffer get_current_command_buffer() const;
    VkFormat get_format() const;
    VkExtent2D get_extent() const;
    VkImageView get_current_frame_image_view() const;

    Frame& get_current_frame();
    u32 get_current_frame_index() const;
    u32 get_current_image_index() const;

    VkSwapchainKHR m_handle;

private:
    GLFWwindow* m_window;
    VkSurfaceKHR m_surface;
    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;

    VkCommandPool m_commandPoll;

    u32 m_currentImageIndex;
    u32 m_imageCount;
    VkImage m_images[FRAME_COUNT];
    VkImageView m_imageViews[FRAME_COUNT];

    u32 m_currentFrameIndex;
    Frame m_frames[FRAME_COUNT];
};

}  // namespace toki

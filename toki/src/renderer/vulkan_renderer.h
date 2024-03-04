#pragma once

#include <array>
#include <vector>

#include "renderer/vulkan_renderer.h"
#include "renderer/vulkan_swapchain.h"
#include "renderer/vulkan_types.h"
#include "toki/renderer/renderer.h"

namespace Toki {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer();

    virtual void init() override;
    virtual void shutdown() override;

    virtual void beginFrame() override;
    virtual void endFrame() override;

    virtual void createSwapchain(Ref<Window> window) override;

private:
    void createInstance();
    void createDevice(VkSurfaceKHR surface);

    void initFrames();
    void initCommandPools();

    void destroyFrames();
    void destroyCommandPools();

    Ref<VulkanContext> m_context;
    std::vector<Ref<VulkanSwapchain>> m_swapchains;

    bool m_wasResized : 1 = false;

    uint8_t m_currentFrame = 0;
    std::array<FrameData, MAX_FRAMES> m_frameData;

    inline static uint8_t s_renderThreadCount = 1;
    inline static uint8_t s_extraCommandPoolCount = 1;

    std::vector<std::array<VkCommandPool, MAX_FRAMES>> m_commandPools;
    std::vector<VkCommandPool> m_extraCommandPools;
};

}  // namespace Toki

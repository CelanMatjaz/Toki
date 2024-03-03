#pragma once

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

    Ref<VulkanContext> m_context;
    std::vector<Ref<VulkanSwapchain>> m_swapchains;
};

}  // namespace Toki

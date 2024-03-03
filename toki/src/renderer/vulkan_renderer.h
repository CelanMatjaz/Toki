#pragma once

#include "renderer/vulkan_types.h"
#include "toki/renderer/renderer.h"

namespace Toki {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer();

    virtual void init() override;
    virtual void shutdown() override;

private:
    void createInstance();
    void createDevice(VkSurfaceKHR surface);

    Ref<VulkanContext> m_context;
};

}  // namespace Toki

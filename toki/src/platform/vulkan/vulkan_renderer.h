#pragma once

#include "core/window.h"
#include "events/events.h"
#include "renderer/renderer.h"
#include "shared_mutex"
#include "vector"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"
#include "vulkan_swapchain.h"

#ifndef MAX_FRAMES
#define MAX_FRAMES 3
#endif

namespace Toki {

struct VulkanRendererConfig {
    uint8_t totalFrames = 3;
};

class VulkanRenderer : public Renderer {
    friend VulkanSwapchain;

public:
    VulkanRenderer(Ref<Window> window);
    virtual ~VulkanRenderer();

    virtual void init() override;
    virtual void shutdown() override;

    virtual bool beginFrame() override;
    virtual void endFrame() override;

    virtual void onEvent(Event& event) override;

    // TODO: remove
    VulkanContext* getContext() { return &context; }

private:
    VulkanContext context;

    void createInstance();
    void createDevice(VkSurfaceKHR surface);

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    std::vector<Ref<VulkanSwapchain>> swapchains;

private:
    bool checkValidationLayerSupport();
    bool checkDeviceExtensionSupport();

    void initFrames();

    FrameData frameData[MAX_FRAMES];
    uint32_t currentFrame = 0;

    static const std::vector<const char*> validationLayers;
    static const std::vector<const char*> requiredExtensions;

    inline static std::shared_mutex rendererLock;
    inline static std::shared_mutex graphicsQueueLock;
    inline static std::shared_mutex presentQueueLock;
};

}  // namespace Toki

#pragma once

#include <memory>

#include "core/macros.h"
#include "engine/window.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/swapchain.h"
#include "renderer_state.h"

namespace toki {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() = delete;
    VulkanRenderer(const Config& config);
    ~VulkanRenderer();

    DELETE_COPY(VulkanRenderer);
    DELETE_MOVE(VulkanRenderer);

public:  // API
    virtual std::shared_ptr<Shader> create_shader(const Shader::Config& config) const override;
private:
    void add_window(std::shared_ptr<Window> window);

    std::vector<std::shared_ptr<Swapchain>> m_swapchains;
};

}  // namespace toki

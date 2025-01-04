#pragma once

#include <memory>

#include "core/macros.h"
#include "engine/window.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/renderer_api.h"
#include "renderer/vulkan/renderer_state.h"
#include "renderer/vulkan/renderer_window.h"
#include "renderer/vulkan/swapchain.h"

namespace toki {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() = delete;
    VulkanRenderer(const Config& config);
    ~VulkanRenderer();

    DELETE_COPY(VulkanRenderer);
    DELETE_MOVE(VulkanRenderer);

public:
    virtual Ref<Shader> create_shader(const Shader::Config& config) const override;

private:
    void add_window(Ref<Window> window);

    void begin_frame() override;
    void end_frame() override;
    void present() override;

    std::vector<Ref<RendererWindow>> m_windows;
};

}  // namespace toki

#pragma once

#include <memory>
#include <unordered_map>

#include "core/id.h"
#include "core/macros.h"
#include "engine/window.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/data/vulkan_shader.h"
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
    virtual Handle create_shader(const Shader::Config& config) override;

private:
    void add_window(Ref<Window> window);

    void begin_frame() override;
    void end_frame() override;
    void present() override;

    std::vector<Ref<RendererWindow>> m_windows;

private:
    std::unordered_map<Handle, Ref<VulkanShader>, Handle> m_shaderMap;
};

}  // namespace toki

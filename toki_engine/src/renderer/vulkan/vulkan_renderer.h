#pragma once

#include <vulkan/vulkan.h>

#include "core/macros.h"
#include "engine/window.h"
#include "renderer/renderer.h"

namespace toki {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer() = delete;
    VulkanRenderer(const Config& config);
    ~VulkanRenderer();

    DELETE_COPY(VulkanRenderer);
    DELETE_MOVE(VulkanRenderer);

public:
    virtual Handle create_shader(const shader_create_config& config) override;
    virtual void destroy_shader(Handle shader_handle) override;

    virtual Handle create_buffer(const buffer_create_config& config) override;
    virtual void destroy_buffer(Handle buffer_handle) override;

    virtual Handle create_framebuffer(const framebuffer_create_config& config) override;
    virtual void destroy_framebuffer(Handle framebuffer_handle) override;

private:
    void add_window(Ref<Window> window);

    virtual b8 begin_frame() override;
    virtual void end_frame() override;
    virtual void present() override;

    virtual void wait_for_resources() override;

private:
    void create_instance();
    void create_device(Ref<Window> window);
    void create_command_pools();
};

}  // namespace toki

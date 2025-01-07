#pragma once

#include <vulkan/vulkan.h>

#include "core/macros.h"
#include "engine/window.h"
#include "renderer/renderer.h"

namespace toki {

class vulkan_renderer : public renderer {
public:
    vulkan_renderer() = delete;
    vulkan_renderer(const Config& config);
    ~vulkan_renderer();

    DELETE_COPY(vulkan_renderer);
    DELETE_MOVE(vulkan_renderer);

public:
    virtual handle create_shader(const shader_create_config& config) override;
    virtual void destroy_shader(handle handle) override;

    virtual handle create_buffer(const buffer_create_config& config) override;
    virtual void destroy_buffer(handle handle) override;

    virtual handle create_framebuffer(const framebuffer_create_config& config) override;
    virtual void destroy_framebuffer(handle handle) override;

private:
    void add_window(ref<window> window);

    b8 begin_frame() override;
    void end_frame() override;
    void present() override;

private:
    void create_instance();
    void create_device(ref<window> window);
    void create_command_pools();
};

}  // namespace toki

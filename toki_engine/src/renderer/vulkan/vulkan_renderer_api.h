#pragma once

#include "renderer/renderer_api.h"
#include "renderer/vulkan/vulkan_context.h"

namespace toki {

class vulkan_renderer_api : public renderer_api {
public:
    vulkan_renderer_api(ref<renderer_context> context);
    ~vulkan_renderer_api();

public:
    virtual void begin_pass(const begin_pass_config& config) override;
    virtual void end_pass() override;
    virtual void submit() override;

    virtual void bind_shader(handle handle) override;

    virtual void reset_viewport() override;
    virtual void reset_scissor() override;

private:
    ref<renderer_context> _context{};

    void fix_render_area(rect2d& render_area);
};

}  // namespace toki

#pragma once

#include "renderer/renderer_api.h"
#include "renderer/vulkan/vulkan_context.h"

namespace toki {

class VulkanRendererApi : public RendererApi {
public:
    VulkanRendererApi(Ref<RendererContext> context);
    ~VulkanRendererApi();

public:
    virtual void begin_pass(const BeginPassConfig& config) override;
    virtual void end_pass() override;
    virtual void submit() override;

    virtual void bind_shader(Handle handle) override;

    virtual void reset_viewport() override;
    virtual void reset_scissor() override;

private:
    Ref<RendererContext> m_context{};

    b8 m_isPassStarted = false;


    void fix_render_area(Rect2D& render_area);
};

}  // namespace toki

#pragma once

#include <memory>

#include "renderer/renderer_api.h"
#include "renderer/vulkan/renderer_state.h"

namespace toki {

class VulkanRendererApi : public RendererApi {
public:
    VulkanRendererApi(Ref<RendererContext> context);
    ~VulkanRendererApi();

public:
    virtual void reset_viewport() const override;
    virtual void reset_scissor() const override;

private:
    Ref<RendererContext> m_context{};
};

}  // namespace toki

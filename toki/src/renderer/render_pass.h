#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "toki/renderer/renderer_types.h"

namespace Toki {

struct RenderPass {
    RenderPass() = default;
    RenderPass(const std::vector<Attachment>& attachments);
    ~RenderPass();

    operator VkRenderPass() { return m_renderPass; }

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
};

}  // namespace Toki

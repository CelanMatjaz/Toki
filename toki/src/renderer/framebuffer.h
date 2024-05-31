#pragma once

#include <vulkan/vulkan.h>

#include "renderer/texture.h"
#include "renderer/vulkan_types.h"
#include "toki/core/core.h"

namespace Toki {

struct Framebuffer {
    Framebuffer() = default;
    Framebuffer(const std::vector<Attachment>& attachments, const Extent3D& extent, Handle swapchainHandle = 0);
    ~Framebuffer();

    void destroy();
    void recreate(const Point3D& extent);

    operator VkFramebuffer();

    std::vector<VkFramebuffer> m_framebuffers;
    std::vector<Ref<Texture>> m_attachmentImages;
    std::vector<Attachment> m_attachments;
    AttachmentFormatHash m_renderPassHandle;
    VkExtent3D m_extent{};

    Handle m_swapchainHandle = 0;
};

}  // namespace Toki

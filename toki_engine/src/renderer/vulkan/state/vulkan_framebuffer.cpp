#include "vulkan_framebuffer.h"

#include <utility>

#include "core/assert.h"
#include "core/logging.h"
#include "renderer/vulkan/vulkan_utils.h"

namespace toki {

void VulkanFramebuffer::create(Ref<RendererContext> ctx, const Config& config) {
    TK_ASSERT(
        config.render_targets.size() > 0 && config.render_targets.size() <= MAX_FRAMEBUFFER_ATTACHMENTS,
        "Provided framebuffer color attachment count is not in range ({} - {})",
        1,
        MAX_FRAMEBUFFER_ATTACHMENTS);

    static_assert((u8) ColorFormat::COLOR_FORMAT_COUNT < 16, "Current render target hashing requires color format enum to not be more than 4 bits long");
    static_assert(MAX_FRAMEBUFFER_ATTACHMENTS <= 8, "Current render target hashing requires at most 8 attachments");

    m_initialRenderTargets = config.render_targets;
    m_colorAttachmentFormats.reserve(MAX_FRAMEBUFFER_ATTACHMENTS);
    m_images.reserve(MAX_FRAMEBUFFER_ATTACHMENTS);

    u32 color_target_index = 0;
    for (u32 i = 0; i < config.render_targets.size(); i++) {
        const RenderTarget& render_target = config.render_targets[i];
        m_initialRenderTargets[i] = render_target;

        if (render_target.presentable) {
            m_presentTargetIndex = i;
            m_colorAttachmentFormats.emplace_back(ctx->swapchain.get_format());
            m_images.emplace_back();
        } else {
            VulkanImage::Config image_config{};
            VkExtent2D swapchain_extent = ctx->swapchain.get_extent();
            image_config.extent = { .width = swapchain_extent.width, .height = swapchain_extent.height, .depth = 1 };
            image_config.format = map_format(render_target.colorFormat);
            image_config.memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            switch (render_target.colorFormat) {
                case ColorFormat::DEPTH:
                case ColorFormat::STENCIL:
                case ColorFormat::DEPTH_STENCIL: {
                    image_config.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    m_depthImage = VulkanImage{};
                    m_depthImage->create(ctx, image_config);
                    break;
                }   
                case ColorFormat::NONE:
                case ColorFormat::COLOR_FORMAT_COUNT:
                    std::unreachable();
                default: {
                    image_config.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
                    VulkanImage image{};
                    image.create(ctx, image_config);
                    m_images.emplace_back(image);
                    break;
                }
            }
        }

        switch (render_target.colorFormat) {
            case ColorFormat::DEPTH:
                m_depthAttachmentFormat = map_format(render_target.colorFormat);
                break;
            case ColorFormat::STENCIL:
                m_stencilAttachmentFormat = map_format(render_target.colorFormat);
                break;
            case ColorFormat::DEPTH_STENCIL:
                m_depthAttachmentFormat = m_stencilAttachmentFormat = map_format(render_target.colorFormat);
                break;
            case ColorFormat::NONE:
            case ColorFormat::COLOR_FORMAT_COUNT:
                std::unreachable();
            default:
                if (!render_target.presentable) {
                    m_colorAttachmentFormats.emplace_back(map_format(render_target.colorFormat));
                }
        }

        TK_ASSERT(m_presentTargetIndex <= 1, "More than one presentable render target provided");
    }
}

void VulkanFramebuffer::destroy(Ref<RendererContext> ctx) {
    for (auto& image : m_images) {
        image.destroy(ctx);
    }

    if (m_depthImage) {
        m_depthImage->destroy(ctx);
    }
}

void VulkanFramebuffer::resize(Ref<RendererContext> ctx, VkExtent3D extent) {
    for (u32 i = 0; i < m_images.size(); i++) {
        if (i == m_presentTargetIndex) {
            continue;
        }
        m_images[i].resize(ctx, extent);
    }

    if (m_depthImage) {
        m_depthImage->resize(ctx, extent);
    }
}

const std::vector<RenderTarget>& VulkanFramebuffer::get_render_target_configs() const {
    return m_initialRenderTargets;
}

const std::vector<VkFormat>& VulkanFramebuffer::get_color_formats() const {
    return m_colorAttachmentFormats;
}

VkFormat VulkanFramebuffer::get_depth_format() const {
    return m_depthAttachmentFormat;
}

VkFormat VulkanFramebuffer::get_stencil_format() const {
    return m_stencilAttachmentFormat;
}

const std::vector<VulkanImage>& VulkanFramebuffer::get_images() const {
    return m_images;
}

const std::optional<VulkanImage>& VulkanFramebuffer::get_depth_image() const {
    return m_depthImage;
}

i32 VulkanFramebuffer::get_present_target_index() const {
    return m_presentTargetIndex;
}

void VulkanFramebuffer::transition_images(VkCommandBuffer cmd) {
    for (u32 i = 0; i < m_images.size(); i++) {
        if (i == m_presentTargetIndex) {
            continue;
        }
        m_images[i].transition_layout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    if (m_depthImage) {
        // m_depthImage->transition_layout(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
    }
}

}  // namespace toki

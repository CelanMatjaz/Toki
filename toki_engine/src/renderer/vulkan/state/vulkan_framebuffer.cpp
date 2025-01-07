#include "vulkan_framebuffer.h"

#include <utility>

#include "core/assert.h"
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

    m_colorAttachmentInfos.reserve(MAX_FRAMEBUFFER_ATTACHMENTS);
    m_attachmentFormats.reserve(MAX_FRAMEBUFFER_ATTACHMENTS);
    m_images.reserve(MAX_FRAMEBUFFER_ATTACHMENTS);

    u32 color_target_index = 0;
    for (u32 i = 0; i < config.render_targets.size(); i++) {
        const RenderTarget& render_target = config.render_targets[i];
        m_initialRenderTargets[i] = render_target;

        VkRenderingAttachmentInfo rendering_attachment_info{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        rendering_attachment_info.loadOp = map_attachment_load_op(render_target.loadOp);
        rendering_attachment_info.storeOp = map_attachment_store_op(render_target.storeOp);

        if (render_target.presentable) {
            m_presentTargetIndex = i;
            m_attachmentFormats.emplace_back(ctx->swapchain.get_format());
            m_images.emplace_back();
        } else {
            VulkanImage::Config image_config{};
            VkExtent2D swapchain_extent = ctx->swapchain.get_extent();
            image_config.extent = { .width = swapchain_extent.width, .height = swapchain_extent.height, .depth = 1 };
            image_config.format = map_format(render_target.colorFormat);

            switch (render_target.colorFormat) {
                case ColorFormat::DEPTH:
                case ColorFormat::STENCIL:
                case ColorFormat::DEPTH_STENCIL:
                    image_config.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    break;
                case ColorFormat::NONE:
                case ColorFormat::COLOR_FORMAT_COUNT:
                    std::unreachable();
                default:
                    image_config.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }

            image_config.memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            VulkanImage image{};
            image.create(ctx, image_config);
            m_images.emplace_back(image);
            rendering_attachment_info.imageView = image.get_image_view();
        }

        switch (render_target.colorFormat) {
            case ColorFormat::DEPTH:
                m_pipelineRenderingCreateInfo.depthAttachmentFormat = map_format(render_target.colorFormat);
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                break;
            case ColorFormat::STENCIL:
                m_pipelineRenderingCreateInfo.stencilAttachmentFormat = map_format(render_target.colorFormat);
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            case ColorFormat::DEPTH_STENCIL:
                m_pipelineRenderingCreateInfo.depthAttachmentFormat = map_format(render_target.colorFormat);
                m_pipelineRenderingCreateInfo.stencilAttachmentFormat = map_format(render_target.colorFormat);
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            case ColorFormat::NONE:
            case ColorFormat::COLOR_FORMAT_COUNT:
                std::unreachable();
            default:
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                if (!render_target.presentable) {
                    m_attachmentFormats.emplace_back(map_format(config.render_targets[i].colorFormat));
                }
                m_colorAttachmentInfos.emplace_back(rendering_attachment_info);
        }

        TK_ASSERT(m_presentTargetIndex <= 1, "More than one presentable render target provided");
    }

    m_pipelineRenderingCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
    m_pipelineRenderingCreateInfo.colorAttachmentCount = color_target_index;
    m_pipelineRenderingCreateInfo.pColorAttachmentFormats = m_attachmentFormats.data();
}

void VulkanFramebuffer::destroy(Ref<RendererContext> ctx) {
    for (auto& image : m_images) {
        image.destroy(ctx);
    }
}

void VulkanFramebuffer::resize(Ref<RendererContext> ctx, VkExtent3D extent) {
    for (u32 i = 0; i < m_images.size(); i++) {
        if (i == m_presentTargetIndex) {
            continue;
        }
        m_images[i].resize(ctx, extent);
        m_colorAttachmentInfos[i].imageView = m_images[i].get_image_view();
    }
}

const std::vector<VkRenderingAttachmentInfo>& VulkanFramebuffer::get_color_attachments_rendering_infos() const {
    return m_colorAttachmentInfos;
}

const VkPipelineRenderingCreateInfoKHR VulkanFramebuffer::get_pipeline_rendering_create_info() const {
    return m_pipelineRenderingCreateInfo;
}

i32 VulkanFramebuffer::get_present_target_index() const {
    return m_presentTargetIndex;
}

}  // namespace toki

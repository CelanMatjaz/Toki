#include "vulkan_framebuffer.h"

#include <utility>

#include "core/assert.h"
#include "renderer/vulkan/vulkan_image.h"
#include "renderer/vulkan/vulkan_utils.h"

namespace toki {

vulkan_framebuffer vulkan_framebuffer_create(ref<renderer_context> ctx, const create_framebuffer_config& config) {
    TK_ASSERT(
        config.render_targets.size() > 0 && config.render_targets.size() <= MAX_FRAMEBUFFER_ATTACHMENTS,
        "Provided framebuffer color attachment count is not in range ({} - {})",
        1,
        MAX_FRAMEBUFFER_ATTACHMENTS);

    vulkan_framebuffer framebuffer{};

    static_assert((u8) color_format::COLOR_FORMAT_COUNT < 16, "Current render target hashing requires color format enum to not be more than 4 bits long");
    static_assert(MAX_FRAMEBUFFER_ATTACHMENTS <= 8, "Current render target hashing requires at most 8 attachments");

    u32 color_target_index = 0;
    for (u32 i = 0; i < config.render_targets.size(); i++) {
        const render_target& render_target = config.render_targets[i];
        framebuffer.initial_render_targets[i] = render_target;

        VkRenderingAttachmentInfo rendering_attachment_info{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        rendering_attachment_info.loadOp = map_attachment_load_op(render_target.load_op);
        rendering_attachment_info.storeOp = map_attachment_store_op(render_target.store_op);

        if (render_target.presentable) {
            framebuffer.present_target_index = i;
            framebuffer.render_target_formats[i] = ctx->swapchain.surface_format.format;
        } else {
            create_image_config image_config{};
            VkExtent2D swapchain_extent = ctx->swapchain.extent;
            image_config.extent = { .width = swapchain_extent.width, .height = swapchain_extent.height, .depth = 1 };
            image_config.format = map_format(render_target.color_format);

            switch (render_target.color_format) {
                case color_format::DEPTH:
                case color_format::STENCIL:
                case color_format::DEPTH_STENCIL:
                    image_config.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
                    break;
                case color_format::NONE:
                case color_format::COLOR_FORMAT_COUNT:
                    std::unreachable();
                default:
                    image_config.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            }

            image_config.memory_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            framebuffer.images[i] = vulkan_image_create(ctx, image_config);
            rendering_attachment_info.imageView = framebuffer.images[i].image_view;
        }

        switch (render_target.color_format) {
            case color_format::DEPTH:
                framebuffer.pipeline_rendering_create_info.depthAttachmentFormat = map_format(render_target.color_format);
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
                break;
            case color_format::STENCIL:
                framebuffer.pipeline_rendering_create_info.stencilAttachmentFormat = map_format(render_target.color_format);
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            case color_format::DEPTH_STENCIL:
                framebuffer.pipeline_rendering_create_info.depthAttachmentFormat = map_format(render_target.color_format);
                framebuffer.pipeline_rendering_create_info.stencilAttachmentFormat = map_format(render_target.color_format);
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            case color_format::NONE:
            case color_format::COLOR_FORMAT_COUNT:
                std::unreachable();
            default:
                rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                if (!render_target.presentable) {
                    framebuffer.render_target_formats[color_target_index] = map_format(config.render_targets[i].color_format);
                }
                framebuffer.color_attachments[color_target_index++] = rendering_attachment_info;
        }

        TK_ASSERT(framebuffer.present_target_index <= 1, "More than one presentable render target provided");
    }

    framebuffer.pipeline_rendering_create_info = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
    framebuffer.pipeline_rendering_create_info.colorAttachmentCount = color_target_index;
    framebuffer.pipeline_rendering_create_info.pColorAttachmentFormats = framebuffer.render_target_formats.data();

    framebuffer.color_render_target_count = color_target_index;

    return framebuffer;
}

void vulkan_framebuffer_destroy(ref<renderer_context> ctx, vulkan_framebuffer& framebuffer) {
    for (auto& image : framebuffer.images) {
        vulkan_image_destroy(ctx, image);
    }
}

void vulkan_framebuffer_resize(ref<renderer_context> ctx, VkExtent3D extent, vulkan_framebuffer& framebuffer) {
    for (u32 i = 0; i < MAX_FRAMEBUFFER_ATTACHMENTS - 1; i++) {
        if (framebuffer.images[i].handle != VK_NULL_HANDLE) {   
            vulkan_image_resize(ctx, extent, framebuffer.images[i]);
            framebuffer.color_attachments[i].imageView = framebuffer.images[i].image_view;
        }
    }
}

}  // namespace toki

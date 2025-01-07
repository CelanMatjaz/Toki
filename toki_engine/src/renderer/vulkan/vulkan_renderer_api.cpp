#include "vulkan_renderer_api.h"

#include "core/assert.h"
#include "renderer/vulkan/vulkan_image.h"
#include "renderer/vulkan/vulkan_types.h"
#include "vulkan/vulkan_core.h"

namespace toki {

vulkan_renderer_api::vulkan_renderer_api(ref<renderer_context> context): _context(context) {}

vulkan_renderer_api::~vulkan_renderer_api() {
    _context.reset();
}

void vulkan_renderer_api::begin_pass(const begin_pass_config& config) {
    TK_ASSERT(_context->framebuffers.contains(config.framebuffer_handle), "Cannot begin render pass without an existing framebuffer");

    rect2d render_area = config.render_area;
    fix_render_area(render_area);

    vulkan_framebuffer& framebuffer = _context->framebuffers.get(config.framebuffer_handle);
    auto& color_attachments = framebuffer.color_attachments;
    auto& color_attachment_count = framebuffer.color_render_target_count;

    if (framebuffer.present_target_index >= 0) {
        color_attachments[framebuffer.present_target_index].imageView = _context->swapchain.image_views[_context->swapchain.current_image_index];
    }

    VkRenderingInfo rendering_info{ VK_STRUCTURE_TYPE_RENDERING_INFO };
    rendering_info.renderArea.extent.width = render_area.size.width;
    rendering_info.renderArea.extent.height = render_area.size.height;
    rendering_info.renderArea.offset.x = render_area.pos.x;
    rendering_info.renderArea.offset.y = render_area.pos.y;
    rendering_info.layerCount = 1;
    rendering_info.pColorAttachments = color_attachments;
    rendering_info.colorAttachmentCount = color_attachment_count;

    vkCmdBeginRendering(_context->get_current_command_buffer(), &rendering_info);
};

void vulkan_renderer_api::end_pass() {
    vkCmdDraw(_context->get_current_command_buffer(), 6, 1, 0, 0);
    vkCmdEndRendering(_context->get_current_command_buffer());
}

void vulkan_renderer_api::submit() {
    vulkan_image image{ .handle = _context->swapchain.images[_context->swapchain.current_frame] };
    vulkan_image_transition_layout(_context->get_current_command_buffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, image);
}

void vulkan_renderer_api::bind_shader(handle shader_handle) {
    TK_ASSERT(_context->shaders.contains(shader_handle), "Shader with provided handle does not exist");
    vulkan_graphics_pipeline& pipeline = _context->shaders.get(shader_handle);
    vkCmdBindPipeline(_context->get_current_command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
}

void vulkan_renderer_api::reset_viewport() {
    Vec2i window_dimensions{ _context->swapchain.extent.width, _context->swapchain.extent.height };

    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = window_dimensions.width;
    viewport.height = window_dimensions.height;
    viewport.minDepth = 0.0f;
    viewport.minDepth = 1.0f;

    vkCmdSetViewport(_context->get_current_command_buffer(), 0, 1, &viewport);
}

void vulkan_renderer_api::reset_scissor() {
    Vec2i window_dimensions{ _context->swapchain.extent.width, _context->swapchain.extent.height };

    VkRect2D scissor{};
    scissor.extent.width = window_dimensions.width;
    scissor.extent.height = window_dimensions.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    vkCmdSetScissor(_context->get_current_command_buffer(), 0, 1, &scissor);
}

void vulkan_renderer_api::fix_render_area(rect2d& render_area) {
    if (render_area.size.width == 0 || render_area.size.height == 0) {
        Vec2i window_dimensions{ _context->swapchain.extent.width, _context->swapchain.extent.height };
        render_area.size.width = window_dimensions.width;
        render_area.size.height = window_dimensions.height;
    }
}

}  // namespace toki

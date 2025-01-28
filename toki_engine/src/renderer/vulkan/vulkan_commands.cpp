#include "vulkan_commands.h"

#include "renderer/vulkan/vulkan_backend.h"
#include "vulkan/vulkan_core.h"

namespace toki {

namespace vulkan_renderer {

#define backend m_backend

VulkanCommands::VulkanCommands(VulkanBackend* b): m_backend(b), m_commandBuffer(b->get_command_buffer()) {}

void VulkanCommands::begin_pass(const Rect2D& render_area) {
    backend->begin_render_pass(m_commandBuffer, render_area);
}

void VulkanCommands::end_pass() {
    backend->end_render_pass(m_commandBuffer);
}

void VulkanCommands::set_viewport(const Rect2D& rect) {
    VkViewport viewport{};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = rect.size.x;
    viewport.height = rect.size.y;
    viewport.minDepth = 0.0;
    viewport.maxDepth = 0.0f;
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);
}

void VulkanCommands::reset_viewport() {
    set_viewport({ { 0, 0 }, { 600.0f, 600.0f } });
}

void VulkanCommands::set_scissor(const Rect2D& rect) {}

void VulkanCommands::reset_scissor() {
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { 600, 600 };
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
}

void VulkanCommands::bind_shader(Shader const& shader) {
    backend->bind_shader(m_commandBuffer, shader);
}

void VulkanCommands::bind_buffer(Buffer const& buffer) {
    backend->bind_buffer(m_commandBuffer, buffer);
}

void VulkanCommands::draw(u32 vertex_count) {
    m_backend->draw(m_commandBuffer, vertex_count);
}

void VulkanCommands::draw_indexed(u32 index_count) {
    m_backend->draw_indexed(m_commandBuffer, index_count);
}

void VulkanCommands::draw_instanced(u32 index_count, u32 instance_count) {
    m_backend->draw_instanced(m_commandBuffer, index_count, instance_count);
}

}  // namespace vulkan_renderer

}  // namespace toki

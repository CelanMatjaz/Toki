#include "vulkan_commands.h"

#include <vulkan/vulkan.h>

#include "vulkan_backend.h"

namespace toki {

#define backend mBackend

VulkanCommands::VulkanCommands(VulkanBackend* b): mBackend(b), mCommandBuffer(b->get_command_buffer()) {}

void VulkanCommands::begin_rendering(const Framebuffer& framebuffer, const Rect2D& render_area) {
	mFramebufferHandle = framebuffer.handle;
	backend->begin_rendering(mCommandBuffer, framebuffer.handle, render_area);
}

void VulkanCommands::end_rendering() {
	backend->end_rendering(mCommandBuffer, mFramebufferHandle);
}

void VulkanCommands::set_viewport(const Rect2D& rect) {
	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (f32) rect.size.x;
	viewport.height = (f32) rect.size.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(mCommandBuffer, 0, 1, &viewport);
}

void VulkanCommands::reset_viewport() {
	set_viewport({ { 0, 0 }, { 600, 600 } });
}

void VulkanCommands::set_scissor(const Rect2D& rect) {
	VkRect2D scissor{};
	scissor.offset = { static_cast<i32>(rect.pos.x), static_cast<i32>(rect.pos.y) };
	scissor.extent = { static_cast<u32>(rect.size.x), static_cast<u32>(rect.size.y) };
	vkCmdSetScissor(mCommandBuffer, 0, 1, &scissor);
}

void VulkanCommands::reset_scissor() {
	set_scissor(Rect2D{ { 0, 0 }, { 600, 600 } });
}

void VulkanCommands::bind_shader(Shader const& shader) {
	mShaderHandle = shader.handle;
	backend->bind_shader(mCommandBuffer, shader);
}

void VulkanCommands::bind_buffer(Buffer const& buffer) {
	backend->bind_buffer(mCommandBuffer, buffer);
}

void VulkanCommands::bind_texture(const Texture& buffer) {}

void VulkanCommands::push_constants(u32 offset, u32 size, const void* data) {
	InternalShader* shader = backend->get_shader(mShaderHandle);
	VkPipelineLayout pipeline_layout = shader->pipeline_layout;
	VkShaderStageFlags shader_stage_flags = shader->push_constant_stage_flags;
	backend->push_constants(mCommandBuffer, pipeline_layout, shader_stage_flags, offset, size, data);
}

void VulkanCommands::draw(u32 vertex_count) {
	mBackend->draw(mCommandBuffer, vertex_count);
}

void VulkanCommands::draw_indexed(u32 index_count) {
	mBackend->draw_indexed(mCommandBuffer, index_count);
}

void VulkanCommands::draw_instanced(u32 index_count, u32 instance_count) {
	mBackend->draw_instanced(mCommandBuffer, index_count, instance_count);
}

}  // namespace toki

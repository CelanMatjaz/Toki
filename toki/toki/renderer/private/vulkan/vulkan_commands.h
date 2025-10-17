#pragma once

#include <toki/renderer/commands.h>

#include "toki/renderer/private/vulkan/vulkan_resources.h"
#include "toki/renderer/private/vulkan/vulkan_state.h"

namespace toki::renderer {

class VulkanCommands : public Commands {
	friend class Renderer;
	friend class VulkanBackend;

public:
	VulkanCommands() = delete;
	VulkanCommands(const VulkanState* state, VulkanCommandBuffer cmd);

public:
	virtual void begin_pass(const BeginPassConfig& config) override;
	virtual void end_pass() override;

	virtual void bind_shader(ShaderHandle handle) override;
	virtual void bind_index_buffer(BufferHandle handle) override;
	virtual void bind_vertex_buffer(BufferHandle handle) override;
	virtual void bind_uniforms(ShaderLayoutHandle handle) override;

	virtual void draw(u32 vertex_count) override;
	virtual void draw_indexed(u32 index_count) override;

private:
	const VulkanState* m_state;
	VulkanCommandBuffer m_cmd;
};

}  // namespace toki::renderer

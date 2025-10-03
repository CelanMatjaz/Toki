#pragma once

#include <toki/renderer/commands.h>

namespace toki::renderer {

class VulkanCommands : public Commands {
	friend class VulkanBackend;

private:
	VulkanCommands() = delete;
	VulkanCommands(VkCommandBuffer cmd): m_cmd(cmd) {}

public:
	virtual void begin_pass() override;
	virtual void end_pass() override;

	virtual void bind_shader(const ShaderHandle& handle) override;
	virtual void draw(u32 vertex_count) override;

private:
	VkCommandBuffer m_cmd;
};

}  // namespace toki::renderer

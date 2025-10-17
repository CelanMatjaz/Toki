#pragma once

#include <toki/core/core.h>
#include <toki/renderer/errors.h>
#include <toki/renderer/frontend/renderer_frontend.h>
#include <toki/renderer/frontend/renderer_types.h>
#include <toki/renderer/private/vulkan/vulkan_commands.h>
#include <toki/renderer/private/vulkan/vulkan_resources.h>
#include <toki/renderer/private/vulkan/vulkan_types.h>
#include <toki/renderer/private/vulkan/vulkan_utils.h>
#include <toki/renderer/types.h>

namespace toki {

class VulkanBackend {
	friend class Renderer;

public:
	VulkanBackend(const RendererConfig& config);
	virtual ~VulkanBackend();

private:
	VulkanBackend() = delete;

	DELETE_COPY(VulkanBackend);
	DELETE_MOVE(VulkanBackend);

private:
	void initialize(const RendererConfig& config);
	void cleanup();

	void initialize_instance();
	void initialize_device(Window* window);

private:
	VulkanState m_state{};
	VulkanSettings m_settings{};
	Window* m_window;
	TempDynamicArray<VulkanCommands*> m_toSubmitCommands;
	// TODO(Matjaž): remove
	VulkanCommands* m_tempCommands;
};

}  // namespace toki

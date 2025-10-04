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

namespace toki::renderer {

class VulkanBackend : public Renderer {
public:
	VulkanBackend() = delete;
	VulkanBackend(const RendererConfig& config);
	~VulkanBackend();

	DELETE_COPY(VulkanBackend);
	DELETE_MOVE(VulkanBackend);

	VulkanBackend* get();

	void attach_window(platform::Window* window);

	virtual void frame_prepare() override;
	virtual void frame_cleanup() override;
	virtual Commands* get_commands() override;
	virtual void submit(Commands*) override;
	virtual void present() override;

	virtual ShaderHandle create_shader(const ShaderConfig& config) override;
	virtual ShaderLayoutHandle create_shader_layout(const ShaderLayoutConfig& config) override;

	virtual void destroy_handle(ShaderHandle) override;
	virtual void destroy_handle(ShaderLayoutHandle) override;

private:
	void initialize(const RendererConfig& config);
	void cleanup();

	void initialize_instance();
	void initialize_device(platform::Window* window);
	void initialize_command_buffers();
	void initialize_frames();

private:
	VulkanState m_state{};
	VulkanSettings m_settings{};
	platform::Window* m_window;
	TempDynamicArray<VulkanCommands*> m_toSubmitCommands;
	// TODO(Matjaž): remove
	toki::UniquePtr<VulkanCommands, RendererPersistentAllocator> m_tempCommands;
};

}  // namespace toki::renderer

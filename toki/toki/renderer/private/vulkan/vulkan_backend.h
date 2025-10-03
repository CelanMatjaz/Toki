#pragma once

#include <toki/core/core.h>
#include <toki/renderer/errors.h>
#include <toki/renderer/private/vulkan/vulkan_commands.h>
#include <toki/renderer/private/vulkan/vulkan_types.h>
#include <toki/renderer/private/vulkan/vulkan_utils.h>
#include <toki/renderer/frontend/renderer_frontend.h>
#include <toki/renderer/frontend/renderer_types.h>
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

	virtual void attach_window(platform::Window* window) override;

	virtual void frame_prepare() override;
	virtual void frame_cleanup() override;
	virtual Commands* get_command_queue_for_frame() override;

	virtual ShaderHandle create_shader(const ShaderConfig& config) override;
	virtual ShaderLayoutHandle create_shader_layout(const ShaderLayoutConfig& config) override;

	virtual void destroy_handle(ShaderHandle) override;
	virtual void destroy_handle(ShaderLayoutHandle) override;

private:
	void cleanup();

	void initialize_instance();
	void initialize_device(platform::Window* window);
	void initialize_command_pool();
	void initialize_command_buffers();
	void initialize_frames();

	// Swapchain functions
	void create_swapchain(const SwapchainConfig& config);
	void recreate_swapchain(WindowState& window_state);
	VkSurfaceKHR create_surface(platform::Window* window);
	void query_present_modes(WindowState& window_state);
	void query_surface_formats(WindowState& window_state);
	void query_surface_extent(WindowState& window_state, VkSurfaceCapabilitiesKHR surface_capabilities);

	// Command buffers
	PersistentDynamicArray<CommandBuffer> allocate_command_buffers(u32 count, VkCommandPool command_pool);

	// Shader functions
	VulkanShader create_vulkan_shader(const ShaderConfig& config);
	VulkanShaderLayout create_vulkan_shader_layout(const ShaderLayoutConfig& config);
	toki::Expected<TempDynamicArray<toki::byte>, RendererErrors> compile_shader(ShaderStage stage, StringView source);
	VkShaderModule create_shader_module(Span<toki::byte> spirv);

	// Image functions
	VkImageView create_image_view(const ImageViewConfig& config);

	VulkanState m_state{};
	VulkanSettings m_settings{};
	VulkanResources m_resources{};
	PersistentDynamicArray<WindowState> m_windowStates;

private:
	static inline VulkanBackend* s_ptr;
};

}  // namespace toki::renderer

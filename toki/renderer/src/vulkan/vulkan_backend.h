#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include "renderer_commands.h"
#include "renderer_types.h"
#include "vulkan_commands.h"
#include "vulkan_types.h"

namespace toki {

class VulkanBackend {
public:
	VulkanBackend();
	~VulkanBackend();

	void device_create();

	void swapchain_create(WeakRef<Window> window);
	void swapchain_destroy();
	void swapchain_recreate(Swapchain& swapchain);

	Framebuffer framebuffer_create(const FramebufferConfig& config);
	void framebuffer_destroy(Framebuffer& framebuffer);

	Buffer buffer_create(const BufferConfig& config);
	void buffer_destroy(Buffer& buffer);
	void buffer_set_data(Buffer buffer, u32 size, void* data);
	void* buffer_map_memory(Buffer buffer, u32 offset, u32 size);
	void buffer_unmap_memory(Buffer buffer);
	void buffer_flush(Buffer buffer);
	void buffer_copy_data(Buffer dst, Buffer src, u32 size, u32 dst_offset = 0, u32 src_offset = 0);

private:
	void* buffer_map_memory(InternalBuffer& internal_buffer, u32 offset, u32 size);
	void buffer_unmap_memory(InternalBuffer& internal_buffer);
	void buffer_copy_data(InternalBuffer& dst, InternalBuffer& src, u32 size, u32 dst_offset = 0, u32 src_offset = 0);

public:
	Texture texture_create(const TextureConfig& config);
	void texture_destroy(Texture& texture_handle);

	Shader shader_create(Framebuffer framebuffer, const ShaderConfig& config);
	void shader_destroy(Shader& shader);
	Handle shader_variant_create(Shader shader, const ShaderVariantConfig& config);

	void resources_initialize();
	void resources_cleanup();

	void resources_wait();

	void prepare_frame_resources();
	void cleanup_frame_resources();
	void submit_commands();
	void present();

	VkCommandBuffer get_command_buffer();
	RendererCommands* get_commands();

	void set_color_clear(const Vec4<f32>& color);
	void set_depth_clear(f32 depth_clear);
	void set_stencil_clear(u32 stencil_clear);

	InternalShader* get_shader(Shader shader);

	// Draw commands
	void begin_rendering(VkCommandBuffer cmd, Framebuffer framebuffer, const Rect2D& render_area);
	void end_rendering(VkCommandBuffer cmd, Framebuffer framebuffer);

	void bind_shader(VkCommandBuffer cmd, Shader shader);
	void bind_buffer(VkCommandBuffer cmd, Buffer buffer);
	void draw(VkCommandBuffer cmd, u32 count);
	void draw_indexed(VkCommandBuffer cmd, u32 count);
	void draw_instanced(VkCommandBuffer cmd, u32 index_count, u32 instance_count = 1);

	void push_constants(
		VkCommandBuffer cmd,
		VkPipelineLayout layout,
		VkShaderStageFlags stage_flags,
		u32 offset,
		u32 size,
		const void* data);

private:
	void create_instance();
	void find_physical_device(VkSurfaceKHR surface);

	InternalBuffer buffer_internal_create(u32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
	void buffer_internal_destroy(InternalBuffer& buffer);

	InternalImage image_internal_create(
		u32 width,
		u32 height,
		u32 layer_count,
		VkFormat format,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags memory_properties,
		VkImageAspectFlags aspect_flags);
	void image_internal_destroy(InternalImage& image);

	void swapchain_prepare_frame(Swapchain& swapchain);
	void swapchain_reset_frame(Swapchain& swapchain);

	b8 submit_frame_command_buffers();
	VkCommandBuffer start_single_use_command_buffer();
	void submit_single_use_command_buffer(VkCommandBuffer cmd);

	FrameData* get_current_frame();
	CommandBuffers& get_current_command_buffers();

	void transition_framebuffer_images(VkCommandBuffer cmd, InternalFramebuffer* framebuffer);
	void transition_swapchain_image(VkCommandBuffer cmd, Swapchain& swapchain);

	void transition_image_layout(const TransitionLayoutConfig& config, InternalImage* image);
	void transition_image_layout(
		VkCommandBuffer cmd, const TransitionLayoutConfig& config, VkImageAspectFlags aspect_flags, VkImage image);
	void transition_image_layout(VkCommandBuffer cmd, const TransitionLayoutConfig& config, InternalImage* image);
	void transition_image_layouts(
		VkCommandBuffer cmd,
		const TransitionLayoutConfig& config,
		VkImageAspectFlags aspect_flags,
		VkImage* images,
		u32 image_count);

	InternalPipeline pipeline_internal_create(
		const InternalFramebuffer& framebuffer, const ShaderVariantConfig& config, VkPipelineLayout pipeline_layout);
	void pipeline_internal_destroy(InternalPipeline& pipeline);
	// PipelineResources create_pipeline_resources(const std::vector<configs::Shader>& stages);
	Span<u32> compile_shader(ShaderStage stage, StringView source_path);

	u32 find_memory_type_index(u32 type_filter, VkMemoryPropertyFlags properties);
	VkImageMemoryBarrier create_image_memory_barrier(
		VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_flags);

	const Limits& limits() const;
	const DeviceProperties& device_properties() const;
	VkImage get_swapchain_image(u32 swapchain_index);
	VkImageView get_swapchain_image_view(u32 swapchain_index);

private:
	VulkanContext mContext;
	VulkanResources mResources;
	VulkanSettings mSettings;
	FrameData mFrames[MAX_FRAMES_IN_FLIGHT];
	u32 mInFlightFrameIndex{};
	DoubleBumpAllocator mFrameAllocator;
};

}  // namespace toki

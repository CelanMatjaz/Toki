#pragma once

#include <toki/renderer/frontend/renderer_types.h>
#include <vulkan/vulkan_core.h>

#include "toki/renderer/private/vulkan/vulkan_types.h"

namespace toki::renderer {

struct VulkanState;
struct VulkanTexture;
struct VulkanCommandBuffer;
struct StagingBufferConfig;
struct WrappedVulkanTexture;

class VulkanSwapchain {
	friend class VulkanFrames;

public:
	static VulkanSwapchain create(const VulkanSwapchainConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void recreate(const VulkanState& state);
	void acquire_next_image(const VulkanState& state);

	VulkanTexture& get_current_image() const;

	operator VkFormat() const {
		return m_surfaceFormat.format;
	}

private:
	VkSwapchainKHR m_swapchain;
	VkSwapchainKHR m_oldSwapchain;
	VkSurfaceKHR m_surface;
	VkExtent2D m_extent;
	VkSurfaceFormatKHR m_surfaceFormat;
	VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
	toki::DynamicArray<WrappedVulkanTexture> m_images;
	PresentModes m_presentModes;
	u32 m_currentImageIndex;
};

class VulkanShaderLayout {
	friend class VulkanShader;

public:
	static VulkanShaderLayout create(const ShaderLayoutConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

private:
	VkPipelineLayout m_pipelineLayout;
};

class VulkanShader {
public:
	static VulkanShader create(const ShaderConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	operator VkPipeline() const {
		return m_pipeline;
	}

private:
	VkPipeline m_pipeline;
};

struct VulkanBufferConfig {
	VulkanBufferConfig() = default;
	VulkanBufferConfig(const BufferConfig& buffer_config): buffer_config(buffer_config) {}

	BufferConfig buffer_config;
	VkBufferUsageFlags override_usage = 0;
	VkMemoryPropertyFlags override_memory_properties = 0;
};

struct VulkanBufferCopyConfig {
	VkBuffer buffer;
	u64 offset;
	u64 size;
};

class VulkanBuffer {
	friend class VulkanStagingBuffer;

public:
	static VulkanBuffer create(const VulkanBufferConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void set_data(const VulkanState& state, const void* data, u64 size);
	void* map_memory(const VulkanState& state);
	void unmap_memory(const VulkanState& state);
	void copy_to_buffer(
		VulkanCommandBuffer cmd, const VulkanBufferCopyConfig& dst_buffer_copy_config = {}, u64 self_offset = 0) const;

	operator VkBuffer() const {
		return m_buffer;
	}

private:
	BufferType m_type;
	VkBuffer m_buffer;
	VkDeviceMemory m_deviceMemory;
	u64 m_size;
};

class VulkanTexture {
	friend class WrappedVulkanTexture;

public:
	static VulkanTexture create(const TextureConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void transition_layout(VulkanCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout);

	operator VkImage() const {
		return m_image;
	}

	operator VkImageView() const {
		return m_imageView;
	}

private:
	VkImage m_image;
	VkImageView m_imageView;
	VkDeviceMemory m_deviceMemory;
};

class WrappedVulkanTexture {
	friend class VulkanSwapchain;

public:
	WrappedVulkanTexture(const VulkanState& state, VkImage image, VkFormat format);
	~WrappedVulkanTexture() = default;

	void destroy(const VulkanState& state);

private:
	VulkanTexture m_texture;
};

class VulkanStagingBuffer {
public:
	static VulkanStagingBuffer create(const StagingBufferConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void set_data_for_buffer(const VulkanState& state, VulkanBuffer& dst_buffer, const void* data, u64 size);
	void reset();

private:
	VulkanBuffer m_buffer;
	u64 m_offset;
	void* m_mappedMemory;
};

struct VulkanCommandBuffer {
	VkCommandBuffer m_commandBuffer;
	CommandBufferState m_state;

	operator VkCommandBuffer() const {
		return m_commandBuffer;
	}

	void begin(const VulkanState& state, VkCommandBufferUsageFlags flags = 0);
	void end(const VulkanState& state);
	void reset(const VulkanState& state);
};

class VulkanCommandPool {
public:
	static VulkanCommandPool create(const VulkanCommandPoolConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	PersistentDynamicArray<VulkanCommandBuffer> allocate_command_buffers(
		const VulkanState& state, u32 count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
	void free_command_buffers(const VulkanState& state, toki::Span<VulkanCommandBuffer> command_buffers) const;

	VulkanCommandBuffer begin_single_time_submit_command_buffer(const VulkanState& state) const;
	void submit_single_time_submit_command_buffer(const VulkanState& state, VulkanCommandBuffer command_buffer) const;

private:
	VkCommandPool m_commandPool;
};

class VulkanFrames {
public:
	static VulkanFrames create(const VulkanState& state);
	void destroy(const VulkanState& state);

	void frame_prepare(VulkanState& state);
	void frame_present(VulkanState& state);
	void frame_cleanup(VulkanState& state);
	b8 submit(const VulkanState& state, toki::Span<VulkanCommandBuffer> command_buffers);

	VkSemaphore get_image_available_semaphore() const;
	VkSemaphore get_render_finished_semaphore() const;
	VkFence get_in_flight_fence() const;

private:
	void increment_frame();

	u32 m_currentFrame{};

	PersistentStaticArray<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
	PersistentStaticArray<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;
	PersistentStaticArray<VkFence, MAX_FRAMES_IN_FLIGHT> m_inFlightFences;
};

}  // namespace toki::renderer

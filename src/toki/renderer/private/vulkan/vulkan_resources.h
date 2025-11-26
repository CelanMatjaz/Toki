#pragma once

#include <toki/renderer/frontend/renderer_types.h>
#include <vulkan/vulkan_core.h>

#include "toki/core/string/span.h"
#include "toki/renderer/private/vulkan/vulkan_types.h"
#include "toki/renderer/types.h"

namespace toki {

struct VulkanState;
struct VulkanTexture;
struct VulkanCommandBuffer;
struct WrappedVulkanTexture;

struct VulkanSwapchain {
	friend struct VulkanFrames;

public:
	static VulkanSwapchain create(const VulkanSwapchainConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void recreate(const VulkanState& state);
	void acquire_next_image(const VulkanState& state);

	VulkanTexture& get_current_image() const;

	VkSwapchainKHR swapchain() const {
		return m_swapchain;
	}

	u32 image_index() const {
		return m_currentImageIndex;
	}

	operator VkFormat() const {
		return m_surfaceFormat.format;
	}

	static void window_listen_function(void* sender, void* listener, const Event& event);

private:
	VkSwapchainKHR m_swapchain;
	VkSwapchainKHR m_oldSwapchain;
	VkSurfaceKHR m_surface;
	VkExtent2D m_extent;
	VkSurfaceFormatKHR m_surfaceFormat;
	VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
	PersistentDynamicArray<WrappedVulkanTexture> m_images;
	PresentModes m_presentModes;
	u32 m_currentImageIndex;
	b32 m_resized;
};

struct VulkanShaderLayout {
	friend struct VulkanShader;

public:
	static VulkanShaderLayout create(const ShaderLayoutConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	VkPipelineLayout layout() const {
		return m_pipelineLayout;
	}

	Span<VkDescriptorSet> sets() {
		return m_descriptorSets;
	}

	void set_descriptors(const VulkanState& state, const SetUniformConfig& config);

private:
	void allocate_descriptor_sets(const VulkanState& state, const AllocateDescriptorSetConfig& config);
	void create_descriptor_set_layout(const VulkanState& state, const DescriptorSetLayoutConfig& config);

	VkPipelineLayout m_pipelineLayout;
	PersistentDynamicArray<VkDescriptorSetLayout> m_descriptorSetLayouts;
	PersistentDynamicArray<VkDescriptorSet> m_descriptorSets;
};

struct VulkanShader {
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
	VulkanBufferConfig(const BufferConfig& config): buffer_config(config) {}

	BufferConfig buffer_config;
	VkBufferUsageFlags override_usage				 = 0;
	VkMemoryPropertyFlags override_memory_properties = 0;
};

struct VulkanBufferCopyConfig {
	VkBuffer buffer;
	u64 offset;
	u64 size;
};

struct VulkanBufferImageCopyConfig {
	VkImage image;
	u32 width;
	u32 height;
};

struct VulkanBuffer {
	friend struct VulkanStagingBuffer;

public:
	static VulkanBuffer create(const VulkanBufferConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void set_data(const VulkanState& state, const void* data, u64 size);
	void* map_memory(const VulkanState& state);
	void unmap_memory(const VulkanState& state);
	void copy_to_buffer(
		VulkanCommandBuffer cmd, const VulkanBufferCopyConfig& dst_buffer_copy_config = {}, u64 self_offset = 0) const;
	void copy_to_image(
		VulkanCommandBuffer cmd,
		const VulkanBufferImageCopyConfig& dst_buffer_copy_config = {},
		u32 self_offset											  = 0) const;

	operator VkBuffer() const {
		return m_buffer;
	}

	VkBuffer buffer() const {
		return m_buffer;
	}

	BufferType type() const {
		return m_type;
	}

	u64 size() const {
		return m_size;
	}

private:
	BufferType m_type;
	VkBuffer m_buffer;
	VkDeviceMemory m_deviceMemory;
	u64 m_size;
};

struct VulkanTexture {
	friend struct WrappedVulkanTexture;

public:
	static VulkanTexture create(const TextureConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void transition_layout(VulkanCommandBuffer cmd, VkImageLayout old_layout, VkImageLayout new_layout);

	VkImage image() const {
		return m_image;
	}

	VkImageView image_view() const {
		return m_imageView;
	}

	u32 width() const {
		return m_metadata.width;
	}

	u32 height() const {
		return m_metadata.height;
	}

	u32 channels() const {
		return m_metadata.channels;
	}

private:
	VkImage m_image;
	VkImageView m_imageView;
	VkDeviceMemory m_deviceMemory;
	TextureConfig m_metadata;
};

struct WrappedVulkanTexture {
	friend struct VulkanSwapchain;

public:
	WrappedVulkanTexture(const VulkanState& state, VkImage image, VkFormat format);
	~WrappedVulkanTexture() = default;

	void destroy(const VulkanState& state);

private:
	VulkanTexture m_texture;
};

struct VulkanSampler {
public:
	static VulkanSampler create(const SamplerConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	operator VkSampler() const {
		return m_sampler;
	}

private:
	VkSampler m_sampler;
};

struct VulkanStagingBuffer {
public:
	static VulkanStagingBuffer create(const StagingBufferConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	void set_data_for_buffer(const VulkanState& state, VulkanBuffer& dst_buffer, const void* data, u64 size);
	void set_data_for_image(const VulkanState& state, VulkanTexture& dst_texture, const void* data, u64 size);
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

	void begin(VkCommandBufferUsageFlags flags = 0);
	void end();
	void reset();
	void free(const VulkanState& state);
};

struct VulkanCommandPool {
public:
	static VulkanCommandPool create(const CommandPoolConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	PersistentDynamicArray<VulkanCommandBuffer> allocate_command_buffers(
		const VulkanState& state, u32 count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
	void free_command_buffers(const VulkanState& state, toki::Span<VulkanCommandBuffer> command_buffers) const;

	VulkanCommandBuffer begin_single_time_submit_command_buffer(const VulkanState& state) const;
	void submit_single_time_submit_command_buffer(const VulkanState& state, VulkanCommandBuffer command_buffer) const;

	VkCommandPool command_pool() const {
		return m_commandPool;
	}

private:
	VkCommandPool m_commandPool;
};

struct VulkanFence {
	static VulkanFence create(b8 signaled, const VulkanState& state);
	void destroy(const VulkanState& state);

	void wait(const VulkanState& state, u64 timeout = U64_MAX);
	void reset(const VulkanState& state);

	VkFence fence() const {
		return m_fence;
	}

	operator VkFence() const {
		return m_fence;
	}

private:
	VkFence m_fence;
};

struct VulkanSemaphore {
	static VulkanSemaphore create(const SemaphoreConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	VkSemaphore semaphore() const {
		return m_semaphore;
	}

	VkPipelineStageFlags stage_flags() const {
		return m_stageFlags;
	}

	operator VkSemaphore() const {
		return m_semaphore;
	}

private:
	VkSemaphore m_semaphore;
	VkPipelineStageFlags m_stageFlags;
};

struct VulkanFrames {
public:
	static VulkanFrames create(VulkanState& state);
	void destroy(const VulkanState& state);

	void frame_prepare(VulkanState& state);
	void frame_cleanup(VulkanState& state);

	VulkanSemaphore get_image_available_semaphore(const VulkanState& state) const;
	VulkanSemaphore get_render_finished_semaphore(const VulkanState& state) const;
	VulkanFence get_in_flight_fence(const VulkanState& state) const;

	SemaphoreHandle get_image_available_semaphore_handle() const;
	SemaphoreHandle get_render_finished_semaphore_handle() const;
	FenceHandle get_in_flight_fence_handle() const;

private:
	void increment_frame();

	u32 m_currentFrame{};

	Array<SemaphoreHandle, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphoreHandles;
	Array<SemaphoreHandle, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphoreHandles;
	Array<FenceHandle, MAX_FRAMES_IN_FLIGHT> m_inFlightFenceHandles;
};

struct VulkanDescriptorPool {
public:
	static VulkanDescriptorPool create(const DescriptorPoolConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	VkDescriptorPool descriptor_pool() const {
		return m_descriptorPool;
	}

private:
	VkDescriptorPool m_descriptorPool;
};

}  // namespace toki

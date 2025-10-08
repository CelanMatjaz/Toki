#pragma once

#include <toki/renderer/frontend/renderer_types.h>
#include <vulkan/vulkan_core.h>

#include "toki/renderer/private/vulkan/vulkan_types.h"
#include "toki/renderer/types.h"

namespace toki::renderer {

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
	PersistentDynamicArray<WrappedVulkanTexture> m_images;
	PresentModes m_presentModes;
	u32 m_currentImageIndex;
};

struct VulkanShaderLayout {
	friend struct VulkanShader;

public:
	static VulkanShaderLayout create(const ShaderLayoutConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

private:
	void create_descriptor_set_layout(const DescriptorSetLayoutConfig& config, const VulkanState& state);

	VkPipelineLayout m_pipelineLayout;
	PersistentDynamicArray<VkDescriptorSetLayout> m_descriptorSetLayouts;
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

private:
	VkCommandPool m_commandPool;
};

struct VulkanFrames {
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

struct VulkanDescriptorPool {
public:
	static VulkanDescriptorPool create(const DescriptorPoolConfig& config, const VulkanState& state);
	void destroy(const VulkanState& state);

	PersistentDynamicArray<VkDescriptorSet> allocate_descriptor_sets(
		const DescriptorSetConfig& config, const VulkanState& state);

private:
	VkDescriptorPool m_descriptorPool;
};

struct SetUniform {
	union {
		BufferHandle buffer;
		TextureHandle texture;
		SamplerHandle sampler;
		struct {
			TextureHandle texture;
			SamplerHandle sampler;
		} texture_with_sampler;
	} handle;
	UniformType type;
	u32 binding;
	u32 array_element;
};

struct SetUniformConfig {};

struct DescriptorSet {
public:
	static DescriptorSet create(const VulkanState& state);

	void write_descriptors(const VulkanState& state, Span<SetUniform> uniforms);

private:
	VkDescriptorSet m_descriptorSet;
};

}  // namespace toki::renderer

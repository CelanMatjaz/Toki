#pragma once

#include <toki/renderer/private/vulkan/vulkan_state.h>
#include <toki/renderer/private/vulkan/vulkan_types.h>

namespace toki::renderer {

VkSurfaceKHR create_surface(const VulkanState& state, Window* window);
PresentModes query_present_modes(const VulkanState& state, VkSurfaceKHR surface);
VkSurfaceFormatKHR query_surface_formats(const VulkanState& state, VkSurfaceKHR surface);
VkExtent2D query_surface_extent(VkSurfaceCapabilitiesKHR surface_capabilities);

VkImageView create_image_view(const ImageViewConfig& config, const VulkanState& state);
VkDeviceMemory allocate_device_memory(const MemoryAllocateConfig& config, const VulkanState& state);
u32 find_memory_type(
	VkPhysicalDeviceMemoryProperties memory_properties, u32 type_filter, VkMemoryPropertyFlags properties);

VkBufferUsageFlags get_buffer_usage_flags(BufferType type);
VkSamplerAddressMode get_address_mode(SamplerAddressMode address_mode);
VkFilter get_filter(SamplerFilter filter);
VkDescriptorType get_descriptor_type(UniformType type);
VkShaderStageFlags get_shader_stage_flags(u32 flags);

}  // namespace toki::renderer

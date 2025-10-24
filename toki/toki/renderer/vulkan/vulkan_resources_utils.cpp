#include "toki/renderer/private/vulkan/vulkan_resources_utils.h"

#include <GLFW/glfw3.h>
#include <toki/core/attributes.h>
#include <toki/renderer/private/vulkan/vulkan_utils.h>
#include <vulkan/vulkan_core.h>

namespace toki {

b8 is_depth_format(VkFormat format) {
	return any_of<
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT_S8_UINT>(format);
}

VkImageView create_image_view(const ImageViewConfig& config, const VulkanState& state) {
	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.image = config.image;
	image_view_create_info.format = config.format;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;

	if (is_depth_format(config.format)) {
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	} else {
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	VkImageView image_view;
	VkResult result =
		vkCreateImageView(state.logical_device, &image_view_create_info, state.allocation_callbacks, &image_view);
	TK_ASSERT(result == VK_SUCCESS);

	return image_view;
}

VkDeviceMemory allocate_device_memory(const MemoryAllocateConfig& config, const VulkanState& state) {
	VkDeviceMemory device_memory;

	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = config.memory_requirements.size;
	memory_allocate_info.memoryTypeIndex = find_memory_type(
		state.physical_device_memory_properties,
		config.memory_requirements.memoryTypeBits,
		config.memory_property_flags);

	VkResult result =
		vkAllocateMemory(state.logical_device, &memory_allocate_info, state.allocation_callbacks, &device_memory);
	TK_ASSERT(result == VK_SUCCESS);

	return device_memory;
}

u32 find_memory_type(
	VkPhysicalDeviceMemoryProperties memory_properties, u32 type_filter, VkMemoryPropertyFlags properties) {
	TK_ASSERT(type_filter != 0 && memory_properties.memoryTypeCount != 0);
	for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	TK_UNREACHABLE();
}

VkBufferUsageFlags get_buffer_usage_flags(BufferType type) {
	switch (type) {
		case BufferType::VERTEX:
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case BufferType::INDEX:
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case BufferType::UNIFORM:
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case BufferType::SIZE:
			break;
	}

	TK_UNREACHABLE();
}

VkImageUsageFlags get_image_usage_flags(u32 texture_flags) {
	VkImageUsageFlags usage_flags = 0;

	if (texture_flags & TextureFlags::WRITABLE) {
		usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	if (texture_flags & TextureFlags::READABLE) {
		usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	if (texture_flags & TextureFlags::SAMPLED) {
		usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	if (texture_flags & TextureFlags::COLOR_ATTACHMENT) {
		usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	if (texture_flags & TextureFlags::DEPTH_STENCIL_ATTACHMENT) {
		usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	return usage_flags;
}

VkSamplerAddressMode get_address_mode(SamplerAddressMode address_mode) {
	switch (address_mode) {
		case SamplerAddressMode::REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case SamplerAddressMode::MIRRORED_REPEAT:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case SamplerAddressMode::CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case SamplerAddressMode::CLAMP_TO_BORDER:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		case SamplerAddressMode::MIRROR_CLAMP_TO_EDGE:
			return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
	}

	TK_UNREACHABLE();
}

VkFilter get_filter(SamplerFilter filter) {
	switch (filter) {
		case SamplerFilter::NEAREST:
			return VK_FILTER_NEAREST;
		case SamplerFilter::LINEAR:
			return VK_FILTER_LINEAR;
	}

	TK_UNREACHABLE();
}

VkSurfaceKHR create_surface(const VulkanState& state, Window* window) {
	VkSurfaceKHR surface;
	VkResult result = VK_RESULT_MAX_ENUM;

#if defined(TK_WINDOW_SYSTEM_GLFW)
	result = glfwCreateWindowSurface(
		state.instance, reinterpret_cast<GLFWwindow*>(window->native_handle()), state.allocation_callbacks, &surface);
#else
	#error "Implement platform specific surface creation"
#endif

	TK_ASSERT(result == VK_SUCCESS);

	return surface;
}

PresentModes query_present_modes(const VulkanState& state, VkSurfaceKHR surface) {
	PresentModes present_modes{};

	u32 present_mode_count{};
	vkGetPhysicalDeviceSurfacePresentModesKHR(state.physical_device, surface, &present_mode_count, nullptr);
	TempDynamicArray<VkPresentModeKHR> queried_present_modes(present_mode_count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		state.physical_device, surface, &present_mode_count, queried_present_modes.data());

	const VkPresentModeKHR present_modes_without_vsync_priorities[2]{ VK_PRESENT_MODE_MAILBOX_KHR,
																	  VK_PRESENT_MODE_IMMEDIATE_KHR };

	const VkPresentModeKHR present_modes_with_vsync_priorities[2]{ VK_PRESENT_MODE_FIFO_KHR,
																   VK_PRESENT_MODE_FIFO_RELAXED_KHR };

	u8 present_mode_indicies[VsyncStatus::VSYNC_STATUS_SIZE] = { static_cast<u8>(-1), static_cast<u8>(-1) };

	for (u8 i = 0; i < queried_present_modes.size(); i++) {
		for (u8 j = 0; j < CARRAY_SIZE(present_modes_with_vsync_priorities); j++) {
			if (queried_present_modes[i] == present_modes_with_vsync_priorities[j] &&
				present_mode_indicies[VSYNC_STATUS_ENABLED] > j) {
				present_mode_indicies[VSYNC_STATUS_ENABLED] = j;
				break;
			}
		}

		for (u8 j = 0; j < CARRAY_SIZE(present_modes_without_vsync_priorities); j++) {
			if (queried_present_modes[i] == present_modes_without_vsync_priorities[j] &&
				present_mode_indicies[VSYNC_STATUS_DISABLED] > j) {
				present_mode_indicies[VSYNC_STATUS_DISABLED] = j;
				break;
			}
		}
	}

	// Check if vsync present mode is supported
	if (present_mode_indicies[VSYNC_STATUS_ENABLED] != static_cast<u8>(-1)) {
		present_modes.modes.vsync_enabled =
			present_modes_with_vsync_priorities[present_mode_indicies[VSYNC_STATUS_ENABLED]];
	}

	present_modes.modes.vsync_disabled =
		present_modes_without_vsync_priorities[present_mode_indicies[VSYNC_STATUS_DISABLED]];

	return present_modes;
}

VkSurfaceFormatKHR query_surface_formats(const VulkanState& state, VkSurfaceKHR surface) {
	u32 surface_format_count{};
	vkGetPhysicalDeviceSurfaceFormatsKHR(state.physical_device, surface, &surface_format_count, nullptr);
	TempDynamicArray<VkSurfaceFormatKHR> surface_formats(surface_format_count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(state.physical_device, surface, &surface_format_count, surface_formats.data());

	for (u32 i = 0; i < surface_formats.size(); ++i) {
		if (surface_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
			surface_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return surface_formats[i];
		}
	}

	return surface_formats[0];
}

VkExtent2D query_surface_extent(VkSurfaceCapabilitiesKHR surface_capabilities) {
	if (surface_capabilities.currentExtent.width != static_cast<u32>(-1)) {
		return surface_capabilities.currentExtent;
	}

	auto calculated_extent = convert_to<VkExtent2D>(Vector2u32{ 400, 400 });
	calculated_extent.width = toki::clamp(
		calculated_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
	calculated_extent.height = toki::clamp(
		calculated_extent.height,
		surface_capabilities.minImageExtent.height,
		surface_capabilities.maxImageExtent.height);

	return calculated_extent;
}

VkDescriptorType get_descriptor_type(UniformType type) {
	switch (type) {
		case UniformType::UNIFORM_BUFFER:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case UniformType::TEXTURE:
			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		case UniformType::SAMPLER:
			return VK_DESCRIPTOR_TYPE_SAMPLER;
		case UniformType::TEXTURE_WITH_SAMPLER:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}

	TK_UNREACHABLE();
}

VkShaderStageFlags get_shader_stage_flags(u32 flags) {
	VkShaderStageFlags flags_out = 0;
	if (flags & ShaderStageFlags::SHADER_STAGE_VERTEX) {
		flags_out |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (flags & ShaderStageFlags::SHADER_STAGE_FRAGMENT) {
		flags_out |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	return flags_out;
}

}  // namespace toki

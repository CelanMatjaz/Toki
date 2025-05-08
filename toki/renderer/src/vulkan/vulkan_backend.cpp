#include "vulkan_backend.h"

#define SHADERC_IMPLEMENTATION
#define SHADERC_DISABLE_CPP_INTERFACE
#include <shaderc/shaderc.h>
#include <toki/core.h>
#include <vulkan/vulkan_core.h>

#include "vulkan_commands.h"
#include "vulkan_platform.h"
#include "vulkan_types.h"

#define ASSERT_EXISTS(resource_array, h, resource)                        \
	TK_ASSERT(                                                            \
		h.handle.valid() && mResources.resource_array.contains(h.handle), \
		"Handle is not associated with any Vulkan " #resource);

#define ASSERT_BUFFER(handle) ASSERT_EXISTS(buffers, handle, buffer)
#define ASSERT_IMAGE(handle) ASSERT_EXISTS(images, handle, image)
#define ASSERT_SHADER(handle) ASSERT_EXISTS(shaders, handle, shader)
#define ASSERT_FRAMEBUFFER(handle) ASSERT_EXISTS(framebuffers, handle, framebuffer)

#define SWAPCHAIN_IMAGE(swapchain) swapchain.images[swapchain.image_index]

namespace toki {

extern WeakRef<Window> window_create_stub(const Window::Config& config);

VulkanBackend::VulkanBackend(): mFrameAllocator(MB(5)) {
	create_instance();
	device_create();
	resources_initialize();
}

VulkanBackend::~VulkanBackend() {
	resources_cleanup();
}

const Limits& VulkanBackend::limits() const {
	return mContext.limits;
}

const DeviceProperties& VulkanBackend::device_properties() const {
	return mContext.properties;
}

static VkFormat get_format(ColorFormat format_in);
static VkFormat get_depth_format(VkPhysicalDevice physical_device, b8 has_stencil);

void VulkanBackend::create_instance() {
	VkApplicationInfo application_info{};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName = "Toki";
	application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	application_info.pEngineName = "Toki Engine";
	application_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	application_info.apiVersion = VK_API_VERSION_1_3;

#if defined(TK_WINDOW_SYSTEM_GLFW)
	u32 count{};
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&count);
	Span extensions{ glfw_extensions, count };

#else
	StaticArray<const char*, 2, BumpAllocator> extensions(mFrameAllocator);
	extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
	#if defined(TK_WINDOW_SYSTEM_WINDOWS)
	extensions[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
	#elif defined(TK_WINDOW_SYSTEM_WAYLAND)
	extensions[1] = VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
	#endif
#endif

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &application_info;

#if !defined(TK_DIST)
	{
		Span validation_layers = mFrameAllocator->allocate_span<const char*>(1);
		validation_layers[0] = "VK_LAYER_KHRONOS_validation";

		b8 layers_supported = true;

		uint32_t layer_count{};
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		Span layers = mFrameAllocator->allocate_span<VkLayerProperties>(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

		for (u32 i = 0; i < validation_layers.size(); i++) {
			const char* required_layer = validation_layers[i];
			b8 layer_found = false;

			for (u32 i = 0; i < layers.size(); i++) {
				const auto& found_layer = layers[i];
				if (toki::strcmp(required_layer, found_layer.layerName)) {
					layer_found = true;
					break;
				}
			}

			if (!layer_found) {
				layers_supported = false;
				break;
			}
		}

		TK_ASSERT(layers_supported, "Validation layers not supported");
		instance_create_info.enabledLayerCount = validation_layers.size();
		instance_create_info.ppEnabledLayerNames = validation_layers.data();
	}
#endif

	instance_create_info.enabledExtensionCount = extensions.size();
	instance_create_info.ppEnabledExtensionNames = extensions.data();

	TK_LOG_INFO("Creating new Vulkan instance");
	VK_CHECK(
		vkCreateInstance(&instance_create_info, mContext.allocation_callbacks, &mContext.instance),
		"Could not initialize renderer");
}

void VulkanBackend::find_physical_device(VkSurfaceKHR surface) {
	u32 physical_device_count{};
	vkEnumeratePhysicalDevices(mContext.instance, &physical_device_count, nullptr);
	TK_ASSERT(physical_device_count > 0, "No GPUs found");

	Span physical_devices = mFrameAllocator->allocate_span<VkPhysicalDevice>(physical_device_count);
	vkEnumeratePhysicalDevices(mContext.instance, &physical_device_count, physical_devices);

	VkPhysicalDeviceProperties device_properties{};
	VkPhysicalDeviceFeatures device_features{};

	mContext.physical_device = physical_devices[0];

	mContext.limits.max_framebuffer_width = device_properties.limits.maxFramebufferWidth;
	mContext.limits.max_framebuffer_height = device_properties.limits.maxFramebufferHeight;
	mContext.limits.max_push_constant_size = device_properties.limits.maxPushConstantsSize;
	mContext.limits.max_color_attachments = device_properties.limits.maxColorAttachments;

	mContext.properties.depth_format = get_depth_format(mContext.physical_device, false);
	mContext.properties.depth_stencil_format = get_depth_format(mContext.physical_device, true);

	u32 queue_family_count{};
	vkGetPhysicalDeviceQueueFamilyProperties(mContext.physical_device, &queue_family_count, nullptr);
	TK_ASSERT(queue_family_count > 0, "No queue families on device");

	Span queue_families = mFrameAllocator->allocate_span<VkQueueFamilyProperties>(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(mContext.physical_device, &queue_family_count, queue_families);

	for (u32 i = 0; i < queue_family_count; i++) {
		VkBool32 supports_present = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(mContext.physical_device, i, surface, &supports_present);
		if (supports_present && mContext.present_queue.family_index == -1) {
			mContext.present_queue.family_index = i;
		}

		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && mContext.graphics_queue.family_index == -1) {
			mContext.graphics_queue.family_index = i;
		}
	}

	{
		u32 present_mode_count{};
		vkGetPhysicalDeviceSurfacePresentModesKHR(mContext.physical_device, surface, &present_mode_count, nullptr);
		TK_ASSERT(present_mode_count > 0, "No present modes found on physical device");

		Span present_modes = mFrameAllocator->allocate_span<VkPresentModeKHR>(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			mContext.physical_device, surface, &present_mode_count, present_modes);

		constexpr VkPresentModeKHR NON_VSYNC_PRESENT_MODES[] = { VK_PRESENT_MODE_MAILBOX_KHR,
																 VK_PRESENT_MODE_IMMEDIATE_KHR };

		mSettings.vsync_disabled_present_mode = VK_PRESENT_MODE_FIFO_KHR;
		for (u32 i = 0; i < present_modes.size(); i++) {
			for (u32 j = 0; j < ARRAY_SIZE(NON_VSYNC_PRESENT_MODES); j++) {
				if (present_modes[i] == NON_VSYNC_PRESENT_MODES[j]) {
					mSettings.vsync_disabled_present_mode = present_modes[i];
					mSettings.non_vsync_supported = true;
					break;
				}
			}
		}
	}

	TK_ASSERT(mContext.present_queue.family_index != 1, "No queue family that supports presenting found");
	TK_ASSERT(mContext.graphics_queue.family_index != 1, "No queue family that supports graphics found");
}

void VulkanBackend::device_create() {
	auto window = window_create_stub({ .title = "", .width = 1, .height = 1 });
	VkSurfaceKHR surface =
		vulkan_surface_create(mContext.instance, mContext.allocation_callbacks, window->get_native_window());

	find_physical_device(surface);

	f32 queue_priority = 1.0f;
	VkDeviceQueueCreateInfo queue_create_infos[2]{};
	queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_infos[0].queueFamilyIndex = mContext.graphics_queue.family_index;
	queue_create_infos[0].queueCount = 1;
	queue_create_infos[0].pQueuePriorities = &queue_priority;
	queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_infos[1].queueFamilyIndex = mContext.present_queue.family_index;
	queue_create_infos[1].queueCount = 1;
	queue_create_infos[1].pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures features{};
	features.fillModeNonSolid = VK_TRUE;
	features.samplerAnisotropy = VK_TRUE;

	VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature{};
	dynamic_rendering_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
	dynamic_rendering_feature.dynamicRendering = VK_TRUE;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = &dynamic_rendering_feature;
	device_create_info.pQueueCreateInfos = queue_create_infos;
	device_create_info.queueCreateInfoCount = 1;
	device_create_info.enabledExtensionCount = sizeof(vulkan_extensions) / sizeof(const char*);
	device_create_info.ppEnabledExtensionNames = vulkan_extensions;
	device_create_info.pEnabledFeatures = &features;

	TK_LOG_INFO("Creating new Vulkan device");
	VK_CHECK(
		vkCreateDevice(mContext.physical_device, &device_create_info, mContext.allocation_callbacks, &mContext.device),
		"Could not create Vulkan device");

	vkGetDeviceQueue(mContext.device, mContext.graphics_queue.family_index, 0, &mContext.graphics_queue.handle);
	vkGetDeviceQueue(mContext.device, mContext.present_queue.family_index, 0, &mContext.present_queue.handle);

	vkDestroySurfaceKHR(mContext.instance, surface, mContext.allocation_callbacks);
	window_destroy(window);
}

static VkSurfaceFormatKHR get_surface_format(VkSurfaceFormatKHR* formats, u32 format_count) {
	for (u32 i = 0; i < format_count; i++) {
		const auto& [format, color_space] = formats[i];
		if (format == VK_FORMAT_B8G8R8A8_SRGB && color_space == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return formats[i];
		}
	}

	return formats[0];
}

static VkPresentModeKHR get_disabled_vsync_present_mode(VkPresentModeKHR* present_modes, u32 mode_count) {
	VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (u32 i = 0; i < mode_count; i++) {
		switch (present_modes[i]) {
			case VK_PRESENT_MODE_IMMEDIATE_KHR:
				return VK_PRESENT_MODE_IMMEDIATE_KHR;
			case VK_PRESENT_MODE_MAILBOX_KHR:
				present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
				if (present_mode != VK_PRESENT_MODE_MAILBOX_KHR) {
					present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
				}
				break;
			case VK_PRESENT_MODE_FIFO_KHR:
				break;
			default:
				TK_UNREACHABLE();
		}
	}

	return present_mode;
}

static VkExtent2D get_surface_extent(VkSurfaceCapabilitiesKHR* capabilities, Window window) {
	if (capabilities->currentExtent.width != ((~0U))) {
		return capabilities->currentExtent;
	}

	return {};

	// auto dimensions = window_get_dimensions(window);
	//
	// VkExtent2D extent{ dimensions.x, dimensions.y };
	// extent.width = clamp(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
	// extent.height = clamp(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);
	//
	// return extent;
}

void VulkanBackend::swapchain_create(WeakRef<Window> window) {
	TK_LOG_INFO("Creating swapchain");

	Swapchain swapchain{};
	swapchain.window = window;
	swapchain.surface =
		vulkan_surface_create(mContext.instance, mContext.allocation_callbacks, window->get_native_window());

	// Query swapchain surface formats
	{
		u32 format_count{};
		vkGetPhysicalDeviceSurfaceFormatsKHR(mContext.physical_device, swapchain.surface, &format_count, nullptr);
		TK_ASSERT(format_count > 0, "No surface formats found on physical device");

		Span surface_formats = mFrameAllocator->allocate_span<VkSurfaceFormatKHR>(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			mContext.physical_device, swapchain.surface, &format_count, surface_formats);
		swapchain.surface_format = get_surface_format(surface_formats, format_count);
	}

	// Query swapchain image count
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext.physical_device, swapchain.surface, &capabilities);
		u32 image_count = clamp<u32>(MAX_FRAMES_IN_FLIGHT, capabilities.minImageCount, capabilities.maxImageCount);
		swapchain.images.resize(image_count);
		swapchain.image_views.resize(image_count);
	}

	// Create image available semaphores
	{
		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(
				vkCreateSemaphore(
					mContext.device,
					&semaphore_create_info,
					mContext.allocation_callbacks,
					&swapchain.image_available_semaphores[i]),
				"Could not create image available semaphore semaphore");
		}
	}

	swapchain_recreate(swapchain);

	mResources.swapchains.resize(1);
	mResources.swapchains[0] = toki::move(swapchain);
}

void VulkanBackend::swapchain_recreate(Swapchain& swapchain) {
	if (swapchain.image_views.size() > 0) {
		for (u32 i = 0; i < swapchain.image_views.size(); i++) {
			vkDestroyImageView(mContext.device, swapchain.image_views[i], mContext.allocation_callbacks);
		}
	}

	VkSurfaceCapabilitiesKHR capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext.physical_device, swapchain.surface, &capabilities);
	u32 image_count = clamp<u32>(MAX_FRAMES_IN_FLIGHT, capabilities.minImageCount, capabilities.maxImageCount);

	auto dimensions = swapchain.window->dimensions();
	swapchain.extent = VkExtent2D{ dimensions.width, dimensions.height };

	// swapchain.extent = get_surface_extent(&capabilities, swapchain.window_handle);
	// TK_ASSERT(swapchain.extent.width > 0 && swapchain.extent.height > 0, "Surface extent is not of valid size");

	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = swapchain.surface;
	swapchain_create_info.minImageCount = swapchain.images.size();
	swapchain_create_info.imageFormat = swapchain.surface_format.format;
	swapchain_create_info.imageColorSpace = swapchain.surface_format.colorSpace;
	swapchain_create_info.imageExtent = swapchain.extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_create_info.preTransform = capabilities.currentTransform;
	swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode =
		mSettings.vsync_enabled ? VK_PRESENT_MODE_FIFO_KHR : mSettings.vsync_disabled_present_mode;
	swapchain_create_info.clipped = VK_TRUE;
	swapchain_create_info.oldSwapchain = swapchain.handle;

	u32 queue_family_indices[] = { static_cast<u32>(mContext.present_queue.family_index),
								   static_cast<u32>(mContext.graphics_queue.family_index) };

	if (queue_family_indices[0] != queue_family_indices[1]) {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = nullptr;
	}

	VK_CHECK(
		vkCreateSwapchainKHR(mContext.device, &swapchain_create_info, mContext.allocation_callbacks, &swapchain.handle),
		"Could not create swapchain");

	if (swapchain_create_info.oldSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(mContext.device, swapchain_create_info.oldSwapchain, mContext.allocation_callbacks);
	}

	{
		u32 image_count{};
		vkGetSwapchainImagesKHR(mContext.device, swapchain.handle, &image_count, nullptr);
		TK_ASSERT(image_count > 0, "No images found for swapchain");

		swapchain.images.resize(image_count);
		swapchain.image_views.resize(image_count);
		vkGetSwapchainImagesKHR(mContext.device, swapchain.handle, &image_count, swapchain.images);

		VkImageViewCreateInfo image_view_create_info{};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = swapchain.surface_format.format;
		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;

		for (u32 i = 0; i < image_count; i++) {
			image_view_create_info.image = swapchain.images[i];
			VK_CHECK(
				vkCreateImageView(
					mContext.device, &image_view_create_info, mContext.allocation_callbacks, &swapchain.image_views[i]),
				"Could not create swapchain image view");
		}
	}
}

void VulkanBackend::swapchain_destroy() {
	Swapchain& swapchain = mResources.swapchains[0];

	for (u32 i = 0; i < swapchain.images.size(); i++) {
		vkDestroyImageView(mContext.device, swapchain.image_views[i], mContext.allocation_callbacks);
	}

	vkDestroySwapchainKHR(mContext.device, swapchain.handle, mContext.allocation_callbacks);
	vkDestroySurfaceKHR(mContext.instance, swapchain.surface, mContext.allocation_callbacks);
}

Framebuffer VulkanBackend::framebuffer_create(const FramebufferConfig& config) {
	InternalFramebuffer framebuffer{};
	framebuffer.has_depth = config.has_depth_attachment;
	framebuffer.has_stencil = config.has_stencil_attachment;
	framebuffer.attachment_count = config.color_format_count;
	framebuffer.image_color_format = config.color_format;

	TK_ASSERT(
		config.color_format_count <= limits().max_color_attachments,
		"Maximum %ul color attachments supported",
		limits().max_color_attachments);

	framebuffer.color_image = BasicRef<InternalImage>();

	*framebuffer.color_image = image_internal_create(
		config.image_width,
		config.image_height,
		config.color_format_count,
		get_format(config.color_format),
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);

	VkImageAspectFlags depthStenctilAspectFlags = 0;

	if (config.has_depth_attachment) {
		depthStenctilAspectFlags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	if (config.has_stencil_attachment) {
		depthStenctilAspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	if (depthStenctilAspectFlags > 0) {
		framebuffer.depth_stencil_image = BasicRef<InternalImage>();
		*framebuffer.depth_stencil_image = image_internal_create(
			config.image_width,
			config.image_height,
			1,
			get_depth_format(
				mContext.physical_device,
				(depthStenctilAspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT) == VK_IMAGE_ASPECT_STENCIL_BIT),
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthStenctilAspectFlags);
	}

	return { mResources.framebuffers.insert(move(framebuffer)) };
}

void VulkanBackend::framebuffer_destroy(Framebuffer& framebuffer) {
	ASSERT_FRAMEBUFFER(framebuffer);
	InternalFramebuffer& internal_framebuffer = mResources.framebuffers[framebuffer.handle];
	image_internal_destroy(internal_framebuffer.color_image);
	if (internal_framebuffer.depth_stencil_image) {
		image_internal_destroy(*internal_framebuffer.depth_stencil_image.data());
	}
}

Buffer VulkanBackend::buffer_create(const BufferConfig& config) {
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	switch (config.type) {
		case BufferType::VERTEX:
			usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		case BufferType::INDEX:
			usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		default:
			TK_UNREACHABLE();
	}

	return { mResources.buffers.insert(
		move(buffer_internal_create(config.size, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))) };
}

void VulkanBackend::buffer_destroy(Buffer& buffer) {
	ASSERT_BUFFER(buffer);
	InternalBuffer& internal_buffer = mResources.buffers[buffer.handle];
	buffer_internal_destroy(internal_buffer);
	mResources.buffers.invalidate(buffer.handle);
}

void VulkanBackend::buffer_set_data(Buffer buffer, u32 size, void* data) {
	ASSERT_BUFFER(buffer);
	InternalBuffer& internal_buffer = mResources.buffers[buffer.handle];

	// Copy to staging buffer
	void* mapped_data = buffer_map_memory(mResources.staging_buffer, mResources.staging_buffer_offset, size);
	toki::memcpy(data, mapped_data, size);
	buffer_unmap_memory(mResources.staging_buffer);
	mapped_data = nullptr;

	// Copy to uploaded buffer
	buffer_copy_data(internal_buffer, mResources.staging_buffer, size, 0, mResources.staging_buffer_offset);

	mResources.staging_buffer_offset += size;
}

void* VulkanBackend::buffer_map_memory(Buffer buffer, u32 offset, u32 size) {
	ASSERT_BUFFER(buffer);
	InternalBuffer& internal_buffer = mResources.buffers[buffer.handle];
	return buffer_map_memory(internal_buffer, offset, size);
}

void* VulkanBackend::buffer_map_memory(InternalBuffer& internal_buffer, u32 offset, u32 size) {
	void* data{};
	VK_CHECK(
		vkMapMemory(mContext.device, internal_buffer.memory, offset, size, 0, &data), "Could not map buffer memory");
	return data;
}

void VulkanBackend::buffer_unmap_memory(InternalBuffer& internal_buffer) {
	vkUnmapMemory(mContext.device, internal_buffer.memory);
}

void VulkanBackend::buffer_unmap_memory(Buffer buffer) {
	ASSERT_BUFFER(buffer);
	InternalBuffer& internal_buffer = mResources.buffers[buffer.handle];
	buffer_unmap_memory(internal_buffer);
}

void VulkanBackend::buffer_flush(Buffer buffer) {
	ASSERT_BUFFER(buffer);
	InternalBuffer& internal_buffer = mResources.buffers[buffer.handle];

	if ((internal_buffer.memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
		VkMappedMemoryRange mapped_memory_range{};
		mapped_memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mapped_memory_range.memory = internal_buffer.memory;
		mapped_memory_range.offset = 0;
		mapped_memory_range.size = internal_buffer.size;
		VK_CHECK(vkFlushMappedMemoryRanges(mContext.device, 1, &mapped_memory_range), "Could not flush buffer memory");
	}
}
void VulkanBackend::buffer_copy_data(Buffer dst, Buffer src, u32 size, u32 dst_offset, u32 src_offset) {
	ASSERT_BUFFER(dst);
	InternalBuffer& dst_internal_buffer = mResources.buffers[dst.handle];
	ASSERT_BUFFER(src);
	InternalBuffer& src_internal_buffer = mResources.buffers[src.handle];
	buffer_copy_data(dst_internal_buffer, src_internal_buffer, size, dst_offset, src_offset);
}

void VulkanBackend::buffer_copy_data(
	InternalBuffer& dst, InternalBuffer& src, u32 size, u32 dst_offset, u32 src_offset) {
	VkCommandBuffer cmd = start_single_use_command_buffer();

	VkBufferCopy buffer_copy{};
	buffer_copy.srcOffset = src_offset;
	buffer_copy.dstOffset = dst_offset;
	buffer_copy.size = size;
	vkCmdCopyBuffer(cmd, src.handle, dst.handle, 1, &buffer_copy);

	submit_single_use_command_buffer(cmd);
}

Texture VulkanBackend::texture_create(const TextureConfig& config) {
	InternalImage new_image = image_internal_create(
		config.width,
		config.height,
		1,
		get_format(config.format),
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT);

	return { mResources.images.insert(move(new_image)) };
}

void VulkanBackend::texture_destroy(Texture& texture) {
	ASSERT_IMAGE(texture);
	mResources.images.invalidate(texture.handle);
}

Shader VulkanBackend::shader_create(Framebuffer framebuffer, const ShaderConfig& config) {
	InternalShader new_shader;
	new_shader.type = config.type;
	new_shader.framebuffer = framebuffer;

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.pSetLayouts = nullptr;
	pipeline_layout_create_info.setLayoutCount = 0;
	pipeline_layout_create_info.pPushConstantRanges = nullptr;
	pipeline_layout_create_info.pushConstantRangeCount = 0;

	VK_CHECK(
		vkCreatePipelineLayout(
			mContext.device, &pipeline_layout_create_info, mContext.allocation_callbacks, &new_shader.pipeline_layout),
		"Could not create pipeline layout");

	return { mResources.shaders.insert(move(new_shader)) };
}

void VulkanBackend::shader_destroy(Shader& shader) {
	ASSERT_SHADER(shader);
	InternalShader& internal_shader = mResources.shaders[shader.handle];
	for (u32 i = 0; i < internal_shader.pipelines.size(); i++) {
		vkDestroyPipeline(mContext.device, internal_shader.pipelines[i].handle, mContext.allocation_callbacks);
	}
}

Handle VulkanBackend::shader_variant_create(Shader shader, const ShaderVariantConfig& config) {
	ASSERT_SHADER(shader);
	InternalShader& internal_shader = mResources.shaders[shader.handle];
	ASSERT_FRAMEBUFFER(internal_shader.framebuffer);
	InternalFramebuffer& framebuffer = mResources.framebuffers[internal_shader.framebuffer.handle];

	pipeline_internal_create(framebuffer, config, internal_shader.pipeline_layout);

	return {};
}

// PipelineResources VulkanBackend::create_pipeline_resources(const std::vector<configs::Shader>& _) {
//     PipelineResources resources{};
//
//     u32 push_constant_count = 0;
//     VkPushConstantRange* push_constants = m_frameAllocator->allocate_aligned<VkPushConstantRange>(16);
//     u32 descriptor_set_layout_count = 0;
//     VkDescriptorSetLayout* descriptor_set_layouts = m_frameAllocator->allocate_aligned<VkDescriptorSetLayout>(16);
//
//     VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
//     pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//     pipeline_layout_create_info.setLayoutCount = descriptor_set_layout_count;
//     pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts;
//     pipeline_layout_create_info.pushConstantRangeCount = push_constant_count;
//     pipeline_layout_create_info.pPushConstantRanges = push_constants;
//
//     VK_CHECK(
//         vkCreatePipelineLayout(
//             mContext.device, &pipeline_layout_create_info, mContext.allocation_callbacks,
//             &resources.pipeline_layout),
//         "Could not create pipeline layout");
//
//     return resources;
// }

InternalPipeline VulkanBackend::pipeline_internal_create(
	const InternalFramebuffer& framebuffer, const ShaderVariantConfig& config, VkPipelineLayout pipeline_layout) {
	InternalPipeline pipeline{};

	VkPipelineShaderStageCreateInfo default_pipeline_shader_stage_create_info{};
	default_pipeline_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	default_pipeline_shader_stage_create_info.pName = "main";

	u32 max_shader_stages = static_cast<u32>(ShaderStage::SHADER_STAGE_COUNT);

	Span shader_stage_create_infos = mFrameAllocator->allocate_span<VkPipelineShaderStageCreateInfo>(max_shader_stages);

	Span vertex_binary = compile_shader(ShaderStage::VERTEX, config.source_paths[0]);
	Span fragment_binary = compile_shader(ShaderStage::FRAGMENT, config.source_paths[1]);

	VkShaderModuleCreateInfo shader_module_create_info{};
	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

	VkShaderModule vertex_shader_module{};
	VkShaderModule framgment_shader_module{};

	shader_module_create_info.codeSize = vertex_binary.size();
	shader_module_create_info.pCode = vertex_binary.data();
	vkCreateShaderModule(
		mContext.device, &shader_module_create_info, mContext.allocation_callbacks, &vertex_shader_module);

	shader_module_create_info.codeSize = fragment_binary.size();
	shader_module_create_info.pCode = fragment_binary.data();
	vkCreateShaderModule(
		mContext.device, &shader_module_create_info, mContext.allocation_callbacks, &framgment_shader_module);

	shader_stage_create_infos[0] = default_pipeline_shader_stage_create_info;
	shader_stage_create_infos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stage_create_infos[0].module = vertex_shader_module;
	shader_stage_create_infos[1] = default_pipeline_shader_stage_create_info;
	shader_stage_create_infos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stage_create_infos[1].module = framgment_shader_module;

	Span vertex_binding_descriptions =
		mFrameAllocator->allocate_span<VkVertexInputBindingDescription>(config.binding_count);
	for (u32 i = 0; i < config.binding_count; i++) {
		vertex_binding_descriptions[i].binding = config.vertex_bindings[i].binding;
		vertex_binding_descriptions[i].stride = config.vertex_bindings[i].stride;

		switch (config.vertex_bindings[i].inputRate) {
			case VertexInputRate::VERTEX:
				vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				break;
			case VertexInputRate::INSTANCE:
				vertex_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
				break;
		}
	}

	Span vertex_attribute_descriptions =
		mFrameAllocator->allocate_span<VkVertexInputAttributeDescription>(config.attribute_count);
	for (u32 i = 0; i < config.attribute_count; i++) {
		vertex_attribute_descriptions[i].binding = config.vertex_attributes[i].binding;
		vertex_attribute_descriptions[i].offset = config.vertex_attributes[i].offset;
		vertex_attribute_descriptions[i].location = config.vertex_attributes[i].location;

		switch (config.vertex_attributes[i].format) {
			case VertexFormat::FLOAT1:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32_SFLOAT;
				break;
			case VertexFormat::FLOAT2:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
				break;
			case VertexFormat::FLOAT3:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case VertexFormat::FLOAT4:
				vertex_attribute_descriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			default:
				TK_UNREACHABLE();
		}
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{};
	vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state_create_info.vertexBindingDescriptionCount = vertex_binding_descriptions.size();
	vertex_input_state_create_info.pVertexBindingDescriptions = vertex_binding_descriptions.data();
	vertex_input_state_create_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
	vertex_input_state_create_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{};
	input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

	switch (config.primitive_topology) {
		case PrimitiveTopology::POINT_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
		case PrimitiveTopology::LINE_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case PrimitiveTopology::LINE_STRIP:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case PrimitiveTopology::TRIANGLE_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case PrimitiveTopology::TRIANGLE_STRIP:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		case PrimitiveTopology::TRIANGLE_FAN:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
			break;
		case PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
			break;
		case PrimitiveTopology::PATH_LIST:
			input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			break;
	}

	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info{};
	rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_state_create_info.depthClampEnable = VK_FALSE;
	rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
	rasterization_state_create_info.lineWidth = 1.0f;
	rasterization_state_create_info.depthBiasEnable = VK_FALSE;
	rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
	rasterization_state_create_info.depthBiasClamp = 0.0f;
	rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;

	switch (config.front_face) {
		case FrontFace::CLOCKWISE:
			rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
			break;
		case FrontFace::COUNTER_CLOCKWISE:
			rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			break;
	}

	switch (config.polygon_mode) {
		case PolygonMode::FILL:
			rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
			break;
		case PolygonMode::LINE:
			rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_LINE;
			break;
		case PolygonMode::POINT:
			rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_POINT;
			break;
	}

	switch (config.cull_mode) {
		case CullMode::NONE:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_NONE;
			break;
		case CullMode::FRONT:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_BIT;
			break;
		case CullMode::BACK:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
			break;
		case CullMode::BOTH:
			rasterization_state_create_info.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
			break;
	}

	VkPipelineMultisampleStateCreateInfo multisample_state_create_info{};
	multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state_create_info.sampleShadingEnable = VK_FALSE;
	multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_state_create_info.minSampleShading = 1.0f;
	multisample_state_create_info.pSampleMask = nullptr;
	multisample_state_create_info.alphaToCoverageEnable = VK_FALSE;
	multisample_state_create_info.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info{};
	depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state_create_info.depthTestEnable = VK_FALSE;
	if (config.depth_test_config.valid()) {
		depth_stencil_state_create_info.depthTestEnable = config.depth_test_config.valid() ? VK_TRUE : VK_FALSE;
		depth_stencil_state_create_info.depthWriteEnable = config.depth_test_config->write_enable ? VK_TRUE : VK_FALSE;
		depth_stencil_state_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state_create_info.minDepthBounds = 0.0f;
		depth_stencil_state_create_info.maxDepthBounds = 1.0f;
		depth_stencil_state_create_info.stencilTestEnable = VK_FALSE;
		depth_stencil_state_create_info.front = {};
		depth_stencil_state_create_info.back = {};

		switch (config.depth_test_config->compare_operation) {
			case CompareOp::NEVER:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NEVER;
				break;
			case CompareOp::LESS:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
				break;
			case CompareOp::EQUAL:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_EQUAL;
				break;
			case CompareOp::LESS_OR_EQUAL:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
				break;
			case CompareOp::GREATER:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER;
				break;
			case CompareOp::NOT_EQUAL:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
				break;
			case CompareOp::GREATER_OR_EQUAL:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
				break;
			case CompareOp::ALWAYS:
				depth_stencil_state_create_info.depthCompareOp = VK_COMPARE_OP_ALWAYS;
				break;
		}
	}

	VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
	color_blend_attachment_state.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment_state.blendEnable = VK_TRUE;
	color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

	Span color_blend_attachment_states =
		mFrameAllocator->allocate_span<VkPipelineColorBlendAttachmentState>(framebuffer.attachment_count);

	for (u32 i = 0; i < framebuffer.attachment_count; i++) {
		color_blend_attachment_states[i] = color_blend_attachment_state;
	}

	VkPipelineColorBlendStateCreateInfo color_blend_state_create_info{};
	color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state_create_info.logicOpEnable = VK_FALSE;
	color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
	color_blend_state_create_info.attachmentCount = color_blend_attachment_states.size();
	color_blend_state_create_info.pAttachments = color_blend_attachment_states.data();
	color_blend_state_create_info.blendConstants[0] = 1.0f;
	color_blend_state_create_info.blendConstants[1] = 1.0f;
	color_blend_state_create_info.blendConstants[2] = 1.0f;
	color_blend_state_create_info.blendConstants[3] = 1.0f;

	constexpr VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info{};
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.dynamicStateCount = ARRAY_SIZE(dynamic_states);
	dynamic_state_create_info.pDynamicStates = dynamic_states;

	VkViewport viewport{};
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};

	VkPipelineViewportStateCreateInfo viewport_state_create_info{};
	viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state_create_info.viewportCount = 1;
	viewport_state_create_info.pViewports = &viewport;
	viewport_state_create_info.scissorCount = 1;
	viewport_state_create_info.pScissors = &scissor;

	Span formats = mFrameAllocator->allocate_span<VkFormat>(framebuffer.attachment_count);
	for (u32 i = 0; i < formats.size(); i++) {
		formats[i] = get_format(framebuffer.image_color_format);
	}

	VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{};
	pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipeline_rendering_create_info.colorAttachmentCount = framebuffer.attachment_count;
	pipeline_rendering_create_info.pColorAttachmentFormats = formats;
	if (framebuffer.has_depth) {
		pipeline_rendering_create_info.depthAttachmentFormat = framebuffer.depth_stencil_image->format;
	}
	if (framebuffer.has_stencil) {
		pipeline_rendering_create_info.stencilAttachmentFormat = framebuffer.depth_stencil_image->format;
	}

	VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
	graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphics_pipeline_create_info.pNext = &pipeline_rendering_create_info;
	graphics_pipeline_create_info.stageCount = shader_stage_create_infos.size();
	graphics_pipeline_create_info.pStages = shader_stage_create_infos.data();
	graphics_pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
	graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
	graphics_pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
	graphics_pipeline_create_info.pMultisampleState = &multisample_state_create_info;
	graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
	graphics_pipeline_create_info.pDepthStencilState = &depth_stencil_state_create_info;
	graphics_pipeline_create_info.pViewportState = &viewport_state_create_info;
	graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
	graphics_pipeline_create_info.renderPass = nullptr;
	graphics_pipeline_create_info.subpass = 0;
	graphics_pipeline_create_info.layout = pipeline_layout;

	TK_LOG_INFO("Creating new graphics pipeline");
	VK_CHECK(
		vkCreateGraphicsPipelines(
			mContext.device,
			VK_NULL_HANDLE,
			1,
			&graphics_pipeline_create_info,
			mContext.allocation_callbacks,
			&pipeline.handle),
		"Could not create graphics pipeline");

	for (u32 i = 0; i < shader_stage_create_infos.size(); i++) {
		vkDestroyShaderModule(mContext.device, shader_stage_create_infos[i].module, mContext.allocation_callbacks);
	}

	return pipeline;
}

void VulkanBackend::pipeline_internal_destroy(InternalPipeline& pipeline) {
	vkDestroyPipeline(mContext.device, pipeline.handle, mContext.allocation_callbacks);
}

Span<u32> VulkanBackend::compile_shader(ShaderStage stage, StringView source_path) {
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compile_options_t options = shaderc_compile_options_initialize();
	TK_ASSERT(compiler && options, "Failed to initialize Shaderc");

	shaderc_compile_options_set_target_env(options, shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
	shaderc_compile_options_set_target_spirv(options, shaderc_spirv_version_1_6);

#ifdef TK_DIST
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_performance);
#else
	shaderc_compile_options_set_optimization_level(options, shaderc_optimization_level_zero);
#endif

	shaderc_shader_kind shader_kind{};

	switch (stage) {
		case ShaderStage::VERTEX:
			shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
			break;
		case ShaderStage::FRAGMENT:
			shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
			break;
		default:
			TK_ASSERT(false, "Shader stage not supported");
			TK_UNREACHABLE();
	}

	FileStream stream(source_path, FILE_READ | FILE_APPEND);
	u32 source_size = stream.tell();
	stream.seek(0);

	Span source_data = mFrameAllocator->allocate_span<char>(source_size);
	stream.read(source_data.data(), source_size);

	shaderc_compilation_result_t result =
		shaderc_compile_into_spv(compiler, source_data.data(), source_size, shader_kind, source_path, "main", options);

	const char* spirv_data{};
	u64 spirv_size{};
	void* data{};

	if (auto status = shaderc_result_get_compilation_status(result); status != shaderc_compilation_status_success) {
		TK_LOG_ERROR(
			"Shader compilation failed for file",
			source_path,
			"\n\tCompilation status:",
			status,
			", with",
			shaderc_result_get_num_errors(result),
			"errors\n\t",
			shaderc_result_get_error_message(result));
		goto cleanup_shaderc;
	}

	spirv_data = shaderc_result_get_bytes(result);
	spirv_size = shaderc_result_get_length(result);

	data = mFrameAllocator->allocate(spirv_size);
	toki::memcpy(spirv_data, data, spirv_size);

cleanup_shaderc:
	shaderc_result_release(result);
	shaderc_compiler_release(compiler);
	shaderc_compile_options_release(options);

	return Span<u32>{ reinterpret_cast<u32*>(data), spirv_size / 4 + (spirv_size | 0b11 ? 1 : 0) };
}

/*
void VulkanBackend::reflect_shader(
	ShaderStage stage,
	std::vector<u32>& binary,
	DescriptorBindings& bindings,
	std::vector<VkPushConstantRange>& push_constants) {
	spirv_cross::Compiler compiler(binary);
	const auto& resources = compiler.get_shader_resources();

	VkShaderStageFlags shader_stage{};
	switch (stage) {
		case ShaderStage::Vertex:
			shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case ShaderStage::Fragment:
			shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		default:
			TK_UNREACHABLE();
	}

	for (u32 i = 0; i < resources.push_constant_buffers.size(); i++) {
		auto& element_type = compiler.get_type(resources.push_constant_buffers[i].base_type_id);
		if (push_constants.size() == 1) {
			push_constants[i].stageFlags |= shader_stage;
		} else {
			VkPushConstantRange push_constant{};
			push_constant.size = compiler.get_declared_struct_size(element_type);
			push_constant.offset =
				compiler.get_decoration(resources.push_constant_buffers[i].id, spv::DecorationOffset);
			push_constant.stageFlags = shader_stage;
			push_constants.emplace_back(push_constant);
		}
	}

	struct ResourceDescriptorType {
		VkDescriptorType type;
		spirv_cross::SmallVector<spirv_cross::Resource> resources;
	};

	// Supported descriptor types
	std::vector<ResourceDescriptorType> descriptor_type_arrays = {
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, resources.uniform_buffers },
		{ VK_DESCRIPTOR_TYPE_SAMPLER, resources.separate_samplers },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, resources.separate_images },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, resources.sampled_images },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resources.storage_buffers },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, resources.storage_images },
	};

	for (const auto& [descriptor_type, descriptor_array] : descriptor_type_arrays) {
		for (u32 descriptor_index = 0; descriptor_index < descriptor_array.size(); descriptor_index++) {
			auto& element_type = compiler.get_type(descriptor_array[descriptor_index].base_type_id);

			u32 set_index =
				compiler.get_decoration(descriptor_array[descriptor_index].id, spv::DecorationDescriptorSet);
			TK_ASSERT(
				set_index <= MAX_DESCRIPTOR_SET_COUNT,
				"A maximum of {} descriptor sets supported (indices 0 - {}), found index is {}",
				MAX_DESCRIPTOR_SET_COUNT,
				MAX_DESCRIPTOR_SET_COUNT - 1,
				set_index);
			TK_ASSERT(
				bindings.binding_counts[set_index] <= MAX_DESCRIPTOR_BINDING_COUNT,
				"A maximum of {} descriptor set bindings supported",
				MAX_DESCRIPTOR_BINDING_COUNT);

			u32 binding_index = compiler.get_decoration(descriptor_array[descriptor_index].id, spv::DecorationBinding);

			bindings.bindings[binding_index][set_index].binding = binding_index;
			bindings.bindings[binding_index][set_index].descriptorType = descriptor_type;
			bindings.bindings[binding_index][set_index].stageFlags |= shader_stage;
			bindings.bindings[binding_index][set_index].descriptorCount =
				element_type.array.size() == 0 ? 1 : element_type.array[0];

			bindings.binding_counts[set_index]++;
		}
	}
}*/

void VulkanBackend::swapchain_prepare_frame(Swapchain& swapchain) {
	VkResult result = vkAcquireNextImageKHR(
		mContext.device,
		swapchain.handle,
		UINT64_MAX,
		swapchain.image_available_semaphores[mInFlightFrameIndex],
		VK_NULL_HANDLE,
		&swapchain.image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		swapchain_recreate(swapchain);
		return;
	} else {
		TK_ASSERT(
			result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR || result == VK_NOT_READY,
			"Could not acquire swapchain image");
	}
}

void VulkanBackend::swapchain_reset_frame(Swapchain& swapchain) {}

VkCommandBuffer VulkanBackend::start_single_use_command_buffer() {
	VkCommandBufferAllocateInfo command_buffer_allocate_info{};
	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandPool = mResources.extra_command_pools[0];
	command_buffer_allocate_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(mContext.device, &command_buffer_allocate_info, &command_buffer);

	VkCommandBufferBeginInfo command_buffer_begin_info{};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info), "Could not begin command buffer");

	return command_buffer;
}

void VulkanBackend::submit_single_use_command_buffer(VkCommandBuffer cmd) {
	vkEndCommandBuffer(cmd);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &cmd;

	vkQueueSubmit(mContext.graphics_queue.handle, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(mContext.graphics_queue.handle);
}

FrameData* VulkanBackend::get_current_frame() {
	return &mFrames[mInFlightFrameIndex];
}

CommandBuffers& VulkanBackend::get_current_command_buffers() {
	return mResources.command_buffers;
}

void VulkanBackend::transition_framebuffer_images(VkCommandBuffer cmd, InternalFramebuffer* framebuffer) {
	u32 memory_barrier_count = 1;
	Span image_memory_barriers = mFrameAllocator->allocate_span<VkImageMemoryBarrier>(memory_barrier_count);

	VkImageMemoryBarrier& color_image_memory_barrier = image_memory_barriers[0];
	color_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	color_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_image_memory_barrier.srcQueueFamilyIndex = mContext.graphics_queue.family_index;
	color_image_memory_barrier.dstQueueFamilyIndex = mContext.graphics_queue.family_index;
	color_image_memory_barrier.subresourceRange.aspectMask = framebuffer->color_image->aspect_flags;
	color_image_memory_barrier.subresourceRange.baseMipLevel = 0;
	color_image_memory_barrier.subresourceRange.levelCount = 1;
	color_image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	color_image_memory_barrier.subresourceRange.layerCount = framebuffer->color_image->image_views.size();

	if (framebuffer->depth_stencil_image) {
		memory_barrier_count++;

		VkImageMemoryBarrier& depth_stencil_image_memory_barrier = image_memory_barriers[1];
		depth_stencil_image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		depth_stencil_image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_stencil_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		depth_stencil_image_memory_barrier.srcQueueFamilyIndex = mContext.graphics_queue.family_index;
		depth_stencil_image_memory_barrier.dstQueueFamilyIndex = mContext.graphics_queue.family_index;
		depth_stencil_image_memory_barrier.subresourceRange.aspectMask = framebuffer->depth_stencil_image->aspect_flags;
		depth_stencil_image_memory_barrier.subresourceRange.baseMipLevel = 0;
		depth_stencil_image_memory_barrier.subresourceRange.levelCount = 1;
		depth_stencil_image_memory_barrier.subresourceRange.baseArrayLayer = 0;
		depth_stencil_image_memory_barrier.subresourceRange.layerCount = 1;

		if (framebuffer->has_stencil) {
			depth_stencil_image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		memory_barrier_count,
		image_memory_barriers.data());
}

void VulkanBackend::transition_swapchain_image(VkCommandBuffer cmd, Swapchain& swapchain) {
	VkImageMemoryBarrier image_memory_barrier{};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_memory_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	image_memory_barrier.srcQueueFamilyIndex = mContext.graphics_queue.family_index;
	image_memory_barrier.dstQueueFamilyIndex = mContext.graphics_queue.family_index;
	image_memory_barrier.image = SWAPCHAIN_IMAGE(swapchain);
	image_memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&image_memory_barrier);
}

void VulkanBackend::transition_image_layout(const TransitionLayoutConfig& config, InternalImage* image) {
	VkCommandBuffer cmd = start_single_use_command_buffer();
	transition_image_layout(cmd, config, image);

	submit_single_use_command_buffer(cmd);
}

void VulkanBackend::transition_image_layout(
	VkCommandBuffer cmd, const TransitionLayoutConfig& config, VkImageAspectFlags aspect_flags, VkImage image) {
	VkImageMemoryBarrier image_memory_barrier =
		create_image_memory_barrier(config.old_layout, config.new_layout, aspect_flags);
	image_memory_barrier.image = image;
	vkCmdPipelineBarrier(cmd, config.src_stage, config.dst_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void VulkanBackend::transition_image_layout(
	VkCommandBuffer cmd, const TransitionLayoutConfig& config, InternalImage* image) {
	transition_image_layout(cmd, config, image->aspect_flags, image->handle);
}

void VulkanBackend::transition_image_layouts(
	VkCommandBuffer cmd,
	const TransitionLayoutConfig& config,
	VkImageAspectFlags aspect_flags,
	VkImage* images,
	u32 image_count) {
	VkImageMemoryBarrier image_memory_barrier =
		create_image_memory_barrier(config.old_layout, config.new_layout, aspect_flags);

	Span image_memory_barriers = mFrameAllocator->allocate_span<VkImageMemoryBarrier>(image_count);
	for (u32 i = 0; i < image_count; i++) {
		image_memory_barriers[i] = image_memory_barrier;
		image_memory_barriers[i].image = images[i];
	}

	vkCmdPipelineBarrier(
		cmd,
		config.src_stage,
		config.dst_stage,
		0,
		0,
		nullptr,
		0,
		nullptr,
		image_memory_barriers.size(),
		image_memory_barriers.data());
}

void VulkanBackend::resources_wait() {
	vkDeviceWaitIdle(mContext.device);
}

void VulkanBackend::prepare_frame_resources() {
	FrameData* frame = get_current_frame();

	VkResult result = vkWaitForFences(mContext.device, 1, &frame->render_fence, VK_TRUE, UINT64_MAX);
	TK_ASSERT(result == VK_SUCCESS || result == VK_TIMEOUT, "Failed waiting for fences");

	for (u32 i = 0; i < mResources.swapchains.size(); i++) {
		swapchain_prepare_frame(mResources.swapchains[i]);
	}

	VK_CHECK(vkResetFences(mContext.device, 1, &frame->render_fence), "Could not reset render fence");

	{
		CommandBuffers& command_buffers = get_current_command_buffers();
		for (u32 i = 0; i < command_buffers.used_count; i++) {
			vkResetCommandBuffer(command_buffers.handles[i], 0);
		}
	}
}

void VulkanBackend::submit_commands() {
	CommandBuffers& command_buffers = get_current_command_buffers();
	for (u32 i = 0; i < command_buffers.used_count; i++) {
		vkEndCommandBuffer(command_buffers.handles[i]);
	}

	submit_frame_command_buffers();
}

void VulkanBackend::cleanup_frame_resources() {
	for (u32 i = 0; i < mResources.swapchains.size(); i++) {
		swapchain_reset_frame(mResources.swapchains[i]);
	}

	mInFlightFrameIndex = (mInFlightFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;

	mFrameAllocator.swap();
	mFrameAllocator->clear();

	mResources.staging_buffer_offset = 0;
}

void VulkanBackend::present() {
	u32 swapchain_count = mResources.swapchains.capacity();
	if (swapchain_count == 0) {
		return;
	}

	FrameData* frame = get_current_frame();
	VkSemaphore wait_semaphores[] = { frame->present_semaphore };

	Span swapchain_handles = mFrameAllocator->allocate_span<VkSwapchainKHR>(swapchain_count);
	Span swapchain_image_indices = mFrameAllocator->allocate_span<u32>(swapchain_count);
	Span swapchains = mFrameAllocator->allocate_span<Swapchain*>(swapchain_count);

	u32 user_swapchain_count = 0;
	for (u32 i = 0; i < swapchain_count; i++) {
		Swapchain& swapchain = mResources.swapchains[i];
		swapchain_handles[user_swapchain_count] = swapchain.handle;
		swapchain_image_indices[user_swapchain_count] = swapchain.image_index;
		swapchains[user_swapchain_count] = &swapchain;
		user_swapchain_count++;
	}

	Span results = mFrameAllocator->allocate_span<VkResult>(swapchain_count);

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = ARRAY_SIZE(wait_semaphores);
	present_info.pWaitSemaphores = wait_semaphores;
	present_info.swapchainCount = swapchain_count;
	present_info.pSwapchains = swapchain_handles.data();
	present_info.pImageIndices = swapchain_image_indices.data();
	present_info.pResults = results.data();

	VK_CHECK(vkQueuePresentKHR(mContext.present_queue.handle, &present_info), "Could not present");

	for (u32 i = 0; i < swapchain_count; i++) {
		if (results[i] == VK_ERROR_OUT_OF_DATE_KHR || results[i] == VK_SUBOPTIMAL_KHR) {
			swapchain_recreate(mResources.swapchains[i]);
		}
	}
}

b8 VulkanBackend::submit_frame_command_buffers() {
	FrameData* frame = get_current_frame();

	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signal_semaphores[] = { frame->present_semaphore };
	Span wait_semaphores = mFrameAllocator->allocate_span<VkSemaphore>(mResources.swapchains.size());

	TK_LOG_INFO("CURRENT FRAME", mInFlightFrameIndex);

	// TODO: make this dynamic when more than 1 swapchain is supported
	for (u32 i = 0; i < wait_semaphores.size(); i++) {
		wait_semaphores[i] = mResources.swapchains[i].image_available_semaphores[mInFlightFrameIndex];
	}

	CommandBuffers& command_buffers = get_current_command_buffers();

	// if (command_buffers.used_count == 0) {
	// 	return true;
	// }

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.waitSemaphoreCount = wait_semaphores.size();
	submit_info.pWaitSemaphores = wait_semaphores.data();
	submit_info.signalSemaphoreCount = ARRAY_SIZE(signal_semaphores);
	submit_info.pSignalSemaphores = signal_semaphores;
	submit_info.pCommandBuffers = command_buffers.handles.data();
	submit_info.commandBufferCount = command_buffers.used_count;

	VK_CHECK(
		vkQueueSubmit(mContext.graphics_queue.handle, 1, &submit_info, frame->render_fence),
		"Could not submit for rendering");

	return true;
}

VkCommandBuffer VulkanBackend::get_command_buffer() {
	CommandBuffers& command_buffers = get_current_command_buffers();

	TK_ASSERT(command_buffers.used_count < MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT, "Cannot get another command buffer");
	VkCommandBuffer cmd = command_buffers.handles[command_buffers.used_count++];

	VkCommandBufferBeginInfo command_buffer_begin_info{};
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &command_buffer_begin_info), "Could not begin command buffer");

	return cmd;
}

RendererCommands* VulkanBackend::get_commands() {
	void* ptr = mFrameAllocator->allocate(sizeof(VulkanCommands));
	return emplace<VulkanCommands>(ptr, this);
}

void VulkanBackend::set_color_clear(const Vec4<f32>& c) {
	mSettings.color_clear = { { c.r, c.g, c.b, c.a } };
}

void VulkanBackend::set_depth_clear(f32 depth_clear) {
	mSettings.depth_stencil_clear.depth = depth_clear;
}

void VulkanBackend::set_stencil_clear(u32 stencil_clear) {
	mSettings.depth_stencil_clear.stencil = stencil_clear;
}

InternalShader* VulkanBackend::get_shader(Shader shader) {
	ASSERT_SHADER(shader);
	return &mResources.shaders[shader.handle];
}

void VulkanBackend::begin_rendering(VkCommandBuffer cmd, Framebuffer framebuffer, const Rect2D& a) {
	ASSERT_FRAMEBUFFER(framebuffer);

	InternalFramebuffer& internal_framebuffer = mResources.framebuffers[framebuffer.handle];
	u64 total_attachment_count = internal_framebuffer.color_image->image_views.size();

	VkRenderingAttachmentInfo rendering_attachment_info{};
	rendering_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	rendering_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	rendering_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	rendering_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	rendering_attachment_info.clearValue.color = mSettings.color_clear;

	Span rendering_attachment_infos = mFrameAllocator->allocate_span<VkRenderingAttachmentInfo>(total_attachment_count);

	for (u32 i = 0; i < total_attachment_count; i++) {
		rendering_attachment_infos[i] = rendering_attachment_info;
		rendering_attachment_infos[i].imageView = internal_framebuffer.color_image->image_views[i];
	}

	VkCommandBuffer transition_layout_cmd = start_single_use_command_buffer();
	TransitionLayoutConfig transition_layout_config{};
	transition_layout_config.old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	transition_layout_config.new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	transition_layout_config.src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	transition_layout_config.dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	transition_image_layouts(
		transition_layout_cmd,
		transition_layout_config,
		VK_IMAGE_ASPECT_COLOR_BIT,
		&internal_framebuffer.color_image->handle,
		1);

	VkRenderingInfo rendering_info{};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.layerCount = 1;
	rendering_info.renderArea = VkRect2D{ { a.pos.x, a.pos.y }, { a.size.x, a.size.y } };
	rendering_info.colorAttachmentCount = internal_framebuffer.color_image->image_views.size();
	rendering_info.pColorAttachments = rendering_attachment_infos.data();

	if (internal_framebuffer.depth_stencil_image) {
		VkRenderingAttachmentInfo* depth_stencil_attachment =
			mFrameAllocator->allocate<VkRenderingAttachmentInfo>(sizeof(VkRenderingAttachmentInfo));

		transition_layout_config.new_layout = internal_framebuffer.has_stencil
												  ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
												  : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		transition_layout_config.src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		transition_layout_config.dst_stage =
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

		transition_image_layout(
			transition_layout_cmd, transition_layout_config, internal_framebuffer.depth_stencil_image.data());

		*depth_stencil_attachment = rendering_attachment_info;
		depth_stencil_attachment->imageView = internal_framebuffer.depth_stencil_image->image_views[0];
		depth_stencil_attachment->clearValue.depthStencil = mSettings.depth_stencil_clear;

		rendering_info.pDepthAttachment = depth_stencil_attachment;
		if (internal_framebuffer.has_stencil) {
			rendering_info.pStencilAttachment = depth_stencil_attachment;
			depth_stencil_attachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		} else {
			depth_stencil_attachment->imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}
	}

	submit_single_use_command_buffer(transition_layout_cmd);

	vkCmdBeginRendering(cmd, &rendering_info);
}

void VulkanBackend::end_rendering(VkCommandBuffer cmd, Framebuffer framebuffer) {
	ASSERT_FRAMEBUFFER(framebuffer);

	vkCmdEndRendering(cmd);

	InternalFramebuffer& internal_framebuffer = mResources.framebuffers[framebuffer.handle];
	Swapchain& swapchain = mResources.swapchains[0];

	// TODO: make layer index be dynamic
	VkImageCopy image_copy{};
	image_copy.extent = internal_framebuffer.color_image->extent;

	// Framebuffer color image subresoruce range
	image_copy.srcSubresource.baseArrayLayer = 0;
	image_copy.srcSubresource.mipLevel = 1;
	image_copy.srcSubresource.layerCount = 1;
	image_copy.srcSubresource.aspectMask = internal_framebuffer.color_image->aspect_flags;

	// Swapchain image subresoruce range
	image_copy.dstSubresource.baseArrayLayer = 0;
	image_copy.dstSubresource.mipLevel = 1;
	image_copy.dstSubresource.layerCount = 1;
	image_copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	image_copy.dstOffset = {};
	image_copy.srcOffset = {};

	vkCmdCopyImage(
		cmd,
		internal_framebuffer.color_image->handle,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		SWAPCHAIN_IMAGE(swapchain),
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		1,
		&image_copy);

	transition_swapchain_image(cmd, swapchain);
}

void VulkanBackend::bind_shader(VkCommandBuffer cmd, Shader shader) {
	ASSERT_SHADER(shader);
	InternalPipeline& pipeline = mResources.shaders[shader.handle].pipelines[shader.handle.data];
	vkCmdBindPipeline(cmd, pipeline.bind_point, pipeline.handle);
}

void VulkanBackend::bind_buffer(VkCommandBuffer cmd, Buffer buffer) {
	ASSERT_BUFFER(buffer);
	InternalBuffer& internal_buffer = mResources.buffers[buffer.handle];

	switch (internal_buffer.usage) {
		case VK_BUFFER_USAGE_VERTEX_BUFFER_BIT: {
			VkDeviceSize offsets[] = { 0 };
			VkBuffer buffers[] = { internal_buffer.handle };
			vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
			break;
		}
		case VK_BUFFER_USAGE_INDEX_BUFFER_BIT: {
			VkDeviceSize offset = 0;
			VkBuffer index_buffer = internal_buffer.handle;
			vkCmdBindIndexBuffer(cmd, index_buffer, offset, VK_INDEX_TYPE_UINT32);
			break;
		}
		default:
			TK_ASSERT(false, "Cannot bind buffer with this type");
	}
}

void VulkanBackend::draw(VkCommandBuffer cmd, u32 count) {
	vkCmdDraw(cmd, count, 1, 0, 0);
}

void VulkanBackend::draw_indexed(VkCommandBuffer cmd, u32 count) {
	draw_instanced(cmd, count, 1);
}

void VulkanBackend::draw_instanced(VkCommandBuffer cmd, u32 index_count, u32 instance_count) {
	vkCmdDrawIndexed(cmd, index_count, instance_count, 0, 0, 0);
}

void VulkanBackend::push_constants(
	VkCommandBuffer cmd,
	VkPipelineLayout layout,
	VkShaderStageFlags stage_flags,
	u32 offset,
	u32 size,
	const void* data) {
	vkCmdPushConstants(cmd, layout, stage_flags, offset, size, data);
}

void VulkanBackend::resources_initialize() {
	// Internal buffer creation
	{
		mResources.staging_buffer = buffer_internal_create(
			DEFAULT_STAGING_BUFFER_SIZE,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

	// Command pool creation
	{
		VkCommandPoolCreateInfo command_pool_create_info{};
		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		command_pool_create_info.queueFamilyIndex = mContext.graphics_queue.family_index;
		mResources.command_pools.resize(1);

		VK_CHECK(
			vkCreateCommandPool(
				mContext.device,
				&command_pool_create_info,
				mContext.allocation_callbacks,
				mResources.command_pools.data()),
			"Could not create command pool(s)");

		mResources.extra_command_pools.resize(1);

		VK_CHECK(
			vkCreateCommandPool(
				mContext.device,
				&command_pool_create_info,
				mContext.allocation_callbacks,
				mResources.extra_command_pools.data()),
			"Could not create extra command pool(s)");
	}

	// Command buffer creation
	{
		VkCommandBufferAllocateInfo command_buffer_allocate_info{};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool = mResources.command_pools[0];
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = MAX_IN_FLIGHT_COMMAND_BUFFER_COUNT;

		mResources.command_buffers.used_count = 0;
		mResources.command_buffers.handles = {};

		for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VK_CHECK(
				vkAllocateCommandBuffers(
					mContext.device, &command_buffer_allocate_info, mResources.command_buffers.handles.data()),
				"Could not allocate command buffers");
		}
	}

	// Frame data creation
	{
		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkSemaphoreCreateInfo semaphore_create_info{};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			VK_CHECK(
				vkCreateFence(
					mContext.device, &fence_create_info, mContext.allocation_callbacks, &mFrames[i].render_fence),
				"Could not create render fence");
			VK_CHECK(
				vkCreateSemaphore(
					mContext.device,
					&semaphore_create_info,
					mContext.allocation_callbacks,
					&mFrames[i].present_semaphore),
				"Could not create present semaphore");
		}
	}
}

void VulkanBackend::resources_cleanup() {
	resources_wait();

	// Cleanup frames
	{
		for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			vkDestroyFence(mContext.device, mFrames[i].render_fence, mContext.allocation_callbacks);
			vkDestroySemaphore(mContext.device, mFrames[i].present_semaphore, mContext.allocation_callbacks);
		}
	}

	// Cleanup command pools
	{
		for (u32 i = 0; i < mResources.command_pools.size(); i++) {
			vkDestroyCommandPool(mContext.device, mResources.command_pools[i], mContext.allocation_callbacks);
		}

		for (u32 i = 0; i < mResources.extra_command_pools.size(); i++) {
			vkDestroyCommandPool(mContext.device, mResources.extra_command_pools[i], mContext.allocation_callbacks);
		}
	}

	// Cleanup internal buffers
	buffer_internal_destroy(mResources.staging_buffer);
}

InternalBuffer VulkanBackend::buffer_internal_create(
	u32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties) {
	InternalBuffer internal_buffer{};

	VkBufferCreateInfo buffer_create_info{};
	buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_create_info.size = size;
	buffer_create_info.usage = usage;
	buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(
		vkCreateBuffer(mContext.device, &buffer_create_info, mContext.allocation_callbacks, &internal_buffer.handle),
		"Could not create buffer");

	VkMemoryRequirements memory_requirements;
	vkGetBufferMemoryRequirements(mContext.device, internal_buffer.handle, &memory_requirements);

	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex =
		find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

	VK_CHECK(
		vkAllocateMemory(
			mContext.device, &memory_allocate_info, mContext.allocation_callbacks, &internal_buffer.memory),
		"Could not allocate image memory");
	VK_CHECK(
		vkBindBufferMemory(mContext.device, internal_buffer.handle, internal_buffer.memory, 0),
		"Could not bind buffer memory");

	internal_buffer.size = size;
	internal_buffer.usage = usage;
	internal_buffer.memory_requirements = memory_requirements;
	internal_buffer.memory_property_flags = memory_properties;

	return internal_buffer;
}

void VulkanBackend::buffer_internal_destroy(InternalBuffer& buffer) {
	vkFreeMemory(mContext.device, buffer.memory, mContext.allocation_callbacks);
	vkDestroyBuffer(mContext.device, buffer.handle, mContext.allocation_callbacks);
	buffer = {};
}

InternalImage VulkanBackend::image_internal_create(
	u32 width,
	u32 height,
	u32 layer_count,
	VkFormat format,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags memory_properties,
	VkImageAspectFlags aspect_flags) {
	InternalImage new_image;

	new_image.image_views.resize(layer_count);
	new_image.extent = VkExtent3D{ width, height, 1 };
	new_image.format = format;
	new_image.aspect_flags = aspect_flags;

	VkImageCreateInfo image_create_info{};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = format;
	image_create_info.extent = new_image.extent;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = layer_count;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = usage;

	VK_CHECK(
		vkCreateImage(mContext.device, &image_create_info, mContext.allocation_callbacks, &new_image.handle),
		"Could not create image");

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(mContext.device, new_image.handle, &memory_requirements);

	VkMemoryAllocateInfo memory_allocate_info{};
	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memory_allocate_info.allocationSize = memory_requirements.size;
	memory_allocate_info.memoryTypeIndex =
		find_memory_type_index(memory_requirements.memoryTypeBits, memory_properties);

	VK_CHECK(
		vkAllocateMemory(mContext.device, &memory_allocate_info, mContext.allocation_callbacks, &new_image.memory),
		"Could not allocate image memory");
	VK_CHECK(vkBindImageMemory(mContext.device, new_image.handle, new_image.memory, 0), "Could not bind image memory");

	VkImageViewCreateInfo image_view_create_info{};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = format;
	image_view_create_info.image = new_image.handle;
	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.subresourceRange.aspectMask = aspect_flags;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.layerCount = 1;

	for (u32 i = 0; i < layer_count; i++) {
		image_view_create_info.subresourceRange.baseArrayLayer = i;
		VK_CHECK(
			vkCreateImageView(
				mContext.device, &image_view_create_info, mContext.allocation_callbacks, &new_image.image_views[i]),
			"Could not create image view");
	}

	return new_image;
}

void VulkanBackend::image_internal_destroy(InternalImage& image) {
	vkFreeMemory(mContext.device, image.memory, mContext.allocation_callbacks);
	for (u32 i = 0; i < image.image_views.size(); i++) {
		vkDestroyImageView(mContext.device, image.image_views[i], mContext.allocation_callbacks);
	}
	vkDestroyImage(mContext.device, image.handle, mContext.allocation_callbacks);
}

u32 VulkanBackend::find_memory_type_index(u32 type_filter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(mContext.physical_device, &memory_properties);
	for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
		if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	TK_ASSERT(false, "No correct memory type found");
	TK_UNREACHABLE();
}

static VkFormat get_format(ColorFormat format_in) {
	switch (format_in) {
		case ColorFormat::RGBA8:
			return VK_FORMAT_R8G8B8A8_SRGB;
		case ColorFormat::NONE:
			TK_ASSERT(false, "Color format not provided");
		default:
			TK_ASSERT(false, "TODO: add other formats");
	}

	TK_UNREACHABLE();
};

static VkFormat get_depth_format(VkPhysicalDevice physical_device, b8 has_stencil) {
	VkFormatFeatureFlags format_feature_flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
	constexpr VkFormat DEPTH_FORMATS[] = { VK_FORMAT_D24_UNORM_S8_UINT,
										   VK_FORMAT_D16_UNORM_S8_UINT,
										   VK_FORMAT_D32_SFLOAT_S8_UINT };

	VkFormatProperties format_properties;
	for (u32 i = 0; i < ARRAY_SIZE(DEPTH_FORMATS); i++) {
		vkGetPhysicalDeviceFormatProperties(physical_device, DEPTH_FORMATS[i], &format_properties);

		if ((format_properties.optimalTilingFeatures & format_feature_flags) == format_feature_flags) {
			return DEPTH_FORMATS[i];
		}
	}

	TK_ASSERT(false, "GPU does not support depth/stencil format");
	TK_UNREACHABLE();
}

VkImageMemoryBarrier VulkanBackend::create_image_memory_barrier(
	VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_flags) {
	VkImageMemoryBarrier image_memory_barrier{};
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.oldLayout = old_layout;
	image_memory_barrier.newLayout = new_layout;
	image_memory_barrier.srcQueueFamilyIndex = mContext.graphics_queue.family_index;
	image_memory_barrier.dstQueueFamilyIndex = mContext.graphics_queue.family_index;
	image_memory_barrier.subresourceRange.aspectMask = aspect_flags;
	image_memory_barrier.subresourceRange.baseMipLevel = 0;
	image_memory_barrier.subresourceRange.levelCount = 1;
	image_memory_barrier.subresourceRange.baseArrayLayer = 0;
	image_memory_barrier.subresourceRange.layerCount = 1;

	switch (old_layout) {
		case VK_IMAGE_LAYOUT_UNDEFINED:
			image_memory_barrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		default:
			TK_ASSERT(false, "Image layout transition not supported");
	}

	switch (new_layout) {
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			image_memory_barrier.dstAccessMask = 0;
			break;
		default:
			TK_ASSERT(false, "Image layout transition not supported");
	}

	return image_memory_barrier;
}

}  // namespace toki

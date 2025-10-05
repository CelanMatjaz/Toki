#include "toki/renderer/private/vulkan/vulkan_backend.h"

#include <GLFW/glfw3.h>
#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "toki/core/common/assert.h"
#include "toki/core/utils/bytes.h"
#include "toki/renderer/private/vulkan/vulkan_resources_utils.h"
#include "toki/renderer/types.h"

namespace toki::renderer {

VulkanBackend::VulkanBackend(const RendererConfig& config): Renderer(config) {
	initialize(config);
}

VulkanBackend::~VulkanBackend() {
	cleanup();
}

void VulkanBackend::initialize(const RendererConfig& config) {
	initialize_instance();
	initialize_device(config.window);

	// Important to query device speific data BEFORE creating resources
	vkGetPhysicalDeviceMemoryProperties(m_state.physical_device, &m_state.physical_device_memory_properties);

	m_state.frames = VulkanFrames::create(m_state);

	VulkanSwapchainConfig swapchain_config{};
	swapchain_config.window = config.window;
	m_state.swapchain = VulkanSwapchain::create(swapchain_config, m_state);

	VulkanCommandPoolConfig command_pool_config{};
	m_state.command_pool = VulkanCommandPool::create(command_pool_config, m_state);
	m_state.temporary_command_pool = VulkanCommandPool::create(command_pool_config, m_state);

	StagingBufferConfig staging_buffer_config{};
	staging_buffer_config.size = toki::MB(500);
	m_state.staging_buffer = VulkanStagingBuffer::create(staging_buffer_config, m_state);

	auto command_buffers = m_state.command_pool.allocate_command_buffers(m_state, 1);
	m_tempCommands = make_unique<VulkanCommands>(&m_state, command_buffers[0]);
	m_toSubmitCommands.resize(1);
}

void VulkanBackend::cleanup() {
	vkDeviceWaitIdle(m_state.logical_device);

	m_state.staging_buffer.destroy(m_state);
	m_state.command_pool.destroy(m_state);
	m_state.temporary_command_pool.destroy(m_state);
	m_state.frames.destroy(m_state);
	m_state.swapchain.destroy(m_state);

	vkDestroyDevice(m_state.logical_device, m_state.allocation_callbacks);
	vkDestroyInstance(m_state.instance, m_state.allocation_callbacks);
}

void VulkanBackend::frame_prepare() {
	m_tempCommands->m_cmd.begin(m_state);

	m_state.frames.frame_prepare(m_state);

	m_state.swapchain.get_current_image().transition_layout(
		reinterpret_cast<VulkanCommands*>(get_commands())->m_cmd,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void VulkanBackend::frame_cleanup() {
	m_state.frames.frame_cleanup(m_state);
}

void VulkanBackend::submit(Commands* commands) {
	m_toSubmitCommands[0] = reinterpret_cast<VulkanCommands*>(commands);
}

void VulkanBackend::present() {
	m_state.swapchain.get_current_image().transition_layout(
		reinterpret_cast<VulkanCommands*>(get_commands())->m_cmd,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	reinterpret_cast<VulkanCommands*>(get_commands())->m_cmd.end(m_state);
	VulkanCommandBuffer command_buffers[] = { reinterpret_cast<VulkanCommands*>(get_commands())->m_cmd };

	m_state.frames.submit(m_state, command_buffers);

	if (m_toSubmitCommands.size() == 0) {
		return;
	}

	m_state.frames.frame_present(m_state);
}

Commands* VulkanBackend::get_commands() {
	return m_tempCommands.get();
}

void VulkanBackend::initialize_instance() {
	VkApplicationInfo application_info{};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName = "Toki engine";
	application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	application_info.pEngineName = "Toki";
	application_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	application_info.apiVersion = VK_API_VERSION_1_4;

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &application_info;
	instance_create_info.enabledLayerCount = 0;

	// u32 property_count;
	// vkEnumerateInstanceExtensionProperties(nullptr, &property_count, nullptr);
	// toki::DynamicArray<VkExtensionProperties> extension_properties(property_count);
	// vkEnumerateInstanceExtensionProperties(nullptr, &property_count, extension_properties.data());
	//
	// for (u32 i = 0; i < extension_properties.size(); i++) {
	// 	toki::print("{} {}\n", extension_properties[i].specVersion, extension_properties[i].extensionName);
	// }

#if defined(TK_WINDOW_SYSTEM_GLFW)
	u32 glfw_extension_count{};
	const char* const* glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	TempDynamicArray<const char*> instance_extensions(glfw_extension_count);
	toki::memcpy(instance_extensions.data(), glfw_extensions, instance_extensions.size() * sizeof(const char*));

	instance_create_info.enabledExtensionCount = instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
#else
	instance_create_info.enabledExtensionCount = 0;
	instance_create_info.ppEnabledExtensionNames = nullptr;
#endif

#if !defined(TK_DIST)
	// Setup validation layers

	const char* validation_layers[]{ "VK_LAYER_KHRONOS_validation" };

	u32 layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	TempDynamicArray<VkLayerProperties> layer_properties(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());

	u32 required_layer_count = ARRAY_SIZE(validation_layers);
	for (u32 i = 0; i < ARRAY_SIZE(validation_layers); ++i) {
		// Found all required layers
		if (required_layer_count == 0) {
			break;
		}

		for (u32 j = 0; j < layer_properties.size(); ++j) {
			if (toki::strcmp(validation_layers[i], layer_properties[j].layerName)) {
				--required_layer_count;
				break;
			}
		}
	}

	TK_ASSERT(required_layer_count == 0, "Not all required Vulkan instance layers found");

	instance_create_info.enabledLayerCount = ARRAY_SIZE(validation_layers);
	instance_create_info.ppEnabledLayerNames = validation_layers;
#else
	instance_create_info.enabledLayerCount = 0;
	instance_create_info.ppEnabledLayerNames = nullptr;
#endif

	VkResult result = vkCreateInstance(&instance_create_info, m_state.allocation_callbacks, &m_state.instance);
	TK_ASSERT(result == VK_SUCCESS);
}

void VulkanBackend::initialize_device(platform::Window* window) {
	m_window = window;
	VkSurfaceKHR surface = create_surface(m_state, window);

	u32 physical_device_count;
	vkEnumeratePhysicalDevices(m_state.instance, &physical_device_count, nullptr);
	TempDynamicArray<VkPhysicalDevice> physical_devices(physical_device_count);
	vkEnumeratePhysicalDevices(m_state.instance, &physical_device_count, physical_devices.data());

	VkPhysicalDeviceProperties physical_device_properties;
	for (u32 i = 0; i < physical_devices.size(); ++i) {
		vkGetPhysicalDeviceProperties(physical_devices[i], &physical_device_properties);
	}

	m_state.physical_device = physical_devices[0];

	u32 queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(m_state.physical_device, &queue_family_count, nullptr);
	TempDynamicArray<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		m_state.physical_device, &queue_family_count, queue_family_properties.data());

	VkBool32 supports_present = false;

	m_state.indices[GRAPHICS_FAMILY_INDEX] = static_cast<u32>(-1);
	m_state.indices[PRESENT_FAMILY_INDEX] = static_cast<u32>(-1);

	for (u32 i = 0; i < queue_family_properties.size(); ++i) {
		if (m_state.indices[PRESENT_FAMILY_INDEX] != static_cast<u32>(-1) &&
			m_state.indices[GRAPHICS_FAMILY_INDEX] != static_cast<u32>(-1)) {
			break;
		}

		if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			m_state.indices[PRESENT_FAMILY_INDEX] == static_cast<u32>(-1)) {
			m_state.indices[GRAPHICS_FAMILY_INDEX] = i;
			continue;
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(m_state.physical_device, i, surface, &supports_present);
		if (supports_present && m_state.indices[PRESENT_FAMILY_INDEX] == static_cast<u32>(-1)) {
			m_state.indices[PRESENT_FAMILY_INDEX] = i;
			continue;
		}
	}

	TK_ASSERT(
		m_state.indices[PRESENT_FAMILY_INDEX] != static_cast<u32>(-1) &&
		m_state.indices[GRAPHICS_FAMILY_INDEX] != static_cast<u32>(-1));

	b8 is_same_queue_family = m_state.indices[GRAPHICS_FAMILY_INDEX] == m_state.indices[PRESENT_FAMILY_INDEX];

	f32 queue_priority = 1.0f;

	TempDynamicArray<VkDeviceQueueCreateInfo> queue_create_infos(2);
	for (u32 i = 0; i < queue_create_infos.size(); i++) {
		queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = m_state.indices[i];
		queue_create_infos[i].queueCount = 1;
		queue_create_infos[i].pQueuePriorities = &queue_priority;
	}

	u32 device_extension_count;
	vkEnumerateDeviceExtensionProperties(m_state.physical_device, nullptr, &device_extension_count, nullptr);
	TempDynamicArray<VkExtensionProperties> queried_device_extensions(device_extension_count);
	vkEnumerateDeviceExtensionProperties(
		m_state.physical_device, nullptr, &device_extension_count, queried_device_extensions.data());

	TempStaticArray<const char*, 1> device_extensions(static_cast<const char*>(VK_KHR_SWAPCHAIN_EXTENSION_NAME));
	// for (u32 i = 0; i < device_extensions.size(); i++) {
	// 	toki::println("{}", device_extensions[i]);
	// }

	VkPhysicalDeviceFeatures physical_device_features{};

	VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features{};
	dynamic_rendering_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext = &dynamic_rendering_features;
	device_create_info.pQueueCreateInfos = queue_create_infos.data();
	device_create_info.queueCreateInfoCount = queue_create_infos.size();
	device_create_info.pEnabledFeatures = &physical_device_features;
	device_create_info.enabledExtensionCount = device_extensions.size();
	device_create_info.ppEnabledExtensionNames = device_extensions.data();

	VkResult result = vkCreateDevice(
		m_state.physical_device, &device_create_info, m_state.allocation_callbacks, &m_state.logical_device);
	TK_ASSERT(result == VK_SUCCESS, "Could not create Vulkan device");

	vkDestroySurfaceKHR(m_state.instance, surface, m_state.allocation_callbacks);

	vkGetDeviceQueue(m_state.logical_device, m_state.indices[GRAPHICS_FAMILY_INDEX], 0, &m_state.graphics_queue);
	vkGetDeviceQueue(m_state.logical_device, m_state.indices[PRESENT_FAMILY_INDEX], 0, &m_state.present_queue);
}

ShaderHandle VulkanBackend::create_shader(const ShaderConfig& config) {
	return { m_state.shaders.emplace_at_first(VulkanShader::create(config, m_state)) };
}

ShaderLayoutHandle VulkanBackend::create_shader_layout(const ShaderLayoutConfig& config) {
	return { m_state.shader_layouts.emplace_at_first(VulkanShaderLayout::create(config, m_state)) };
}

void VulkanBackend::destroy_handle(ShaderHandle shader_handle) {
	TK_ASSERT(m_state.shaders.exists(shader_handle));
	m_state.shaders.at(shader_handle).destroy(m_state);
}

void VulkanBackend::destroy_handle(ShaderLayoutHandle shader_layout_handle) {
	TK_ASSERT(m_state.shader_layouts.exists(shader_layout_handle));
	m_state.shader_layouts.at(shader_layout_handle).destroy(m_state);
}

}  // namespace toki::renderer

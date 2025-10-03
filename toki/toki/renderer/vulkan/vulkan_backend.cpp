#include "toki/renderer/private/vulkan/vulkan_backend.h"

#include <GLFW/glfw3.h>
#include <toki/core/core.h>
#include <toki/platform/platform.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "toki/core/common/assert.h"
#include "toki/core/utils/memory.h"

namespace toki::renderer {

VulkanBackend* VulkanBackend::get() {
	return s_ptr;
}

VulkanBackend::VulkanBackend(const RendererConfig& config): Renderer(config) {
	TK_ASSERT(s_ptr == nullptr);
	s_ptr = this;

	initialize_instance();
	initialize_device(config.window);
	initialize_command_pool();
	initialize_command_buffers();
	initialize_frames();
}

VulkanBackend::~VulkanBackend() {
	cleanup();

	s_ptr = nullptr;
}

void VulkanBackend::cleanup() {
	vkDeviceWaitIdle(m_state.device);

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyFence(m_state.device, m_resources.frames.in_flight_fences[i], m_state.allocation_callbacks);
		vkDestroySemaphore(
			m_state.device, m_resources.frames.image_available_semaphores[i], m_state.allocation_callbacks);
		vkDestroySemaphore(
			m_state.device, m_resources.frames.render_finished_semaphores[i], m_state.allocation_callbacks);
	}

	vkDestroyCommandPool(m_state.device, m_resources.temporary_command_pool, m_state.allocation_callbacks);
	vkDestroyCommandPool(m_state.device, m_resources.command_pool, m_state.allocation_callbacks);

	for (u32 i = 0; i < m_windowStates.size(); ++i) {
		m_windowStates[i].cleanup(m_state);
	}

	m_state.cleanup();
}

void VulkanBackend::attach_window(platform::Window* window) {
	TK_ASSERT(window != nullptr);

	m_windowStates.emplace_back({});
	window->set_renderer_pointer(reinterpret_cast<void*>(m_windowStates.size() - 1));

	SwapchainConfig swapchain_config{};
	swapchain_config.window_state_index = m_windowStates.size() - 1;
	swapchain_config.window = window;
	create_swapchain(swapchain_config);
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
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	TempDynamicArray<const char*> instance_extensions(glfw_extension_count);
	toki::memcpy(glfw_extensions, instance_extensions.data(), instance_extensions.size() * sizeof(const char*));

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
	VkSurfaceKHR surface = create_surface(window);

	u32 physical_device_count;
	vkEnumeratePhysicalDevices(m_state.instance, &physical_device_count, nullptr);
	TempDynamicArray<VkPhysicalDevice> physical_devices(physical_device_count);
	vkEnumeratePhysicalDevices(m_state.instance, &physical_device_count, physical_devices.data());

	VkPhysicalDeviceProperties physical_device_properties;
	for (u32 i = 0; i < physical_devices.size(); ++i) {
		vkGetPhysicalDeviceProperties(physical_devices[i], &physical_device_properties);
	}

	m_state.device.physical_device = physical_devices[0];

	u32 queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(m_state.device, &queue_family_count, nullptr);
	TempDynamicArray<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_state.device, &queue_family_count, queue_family_properties.data());

	VkBool32 supports_present = false;

	m_state.device.indices[GRAPHICS_FAMILY_INDEX] = static_cast<u32>(-1);
	m_state.device.indices[PRESENT_FAMILY_INDEX] = static_cast<u32>(-1);

	for (u32 i = 0; i < queue_family_properties.size(); ++i) {
		if (m_state.device.indices[PRESENT_FAMILY_INDEX] != static_cast<u32>(-1) &&
			m_state.device.indices[GRAPHICS_FAMILY_INDEX] != static_cast<u32>(-1)) {
			break;
		}

		if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			m_state.device.indices[PRESENT_FAMILY_INDEX] == static_cast<u32>(-1)) {
			m_state.device.indices[GRAPHICS_FAMILY_INDEX] = i;
			continue;
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(m_state.device, i, surface, &supports_present);
		if (supports_present && m_state.device.indices[PRESENT_FAMILY_INDEX] == static_cast<u32>(-1)) {
			m_state.device.indices[PRESENT_FAMILY_INDEX] = i;
			continue;
		}
	}

	TK_ASSERT(
		m_state.device.indices[PRESENT_FAMILY_INDEX] != static_cast<u32>(-1) &&
		m_state.device.indices[GRAPHICS_FAMILY_INDEX] != static_cast<u32>(-1));

	b8 is_same_queue_family =
		m_state.device.indices[GRAPHICS_FAMILY_INDEX] == m_state.device.indices[PRESENT_FAMILY_INDEX];

	f32 queue_priority = 1.0f;

	TempDynamicArray<VkDeviceQueueCreateInfo> queue_create_infos(2);
	for (u32 i = 0; i < queue_create_infos.size(); i++) {
		queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = m_state.device.indices[i];
		queue_create_infos[i].queueCount = 1;
		queue_create_infos[i].pQueuePriorities = &queue_priority;
	}

	u32 device_extension_count;
	vkEnumerateDeviceExtensionProperties(m_state.device, nullptr, &device_extension_count, nullptr);
	TempDynamicArray<VkExtensionProperties> queried_device_extensions(device_extension_count);
	vkEnumerateDeviceExtensionProperties(
		m_state.device, nullptr, &device_extension_count, queried_device_extensions.data());

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
		m_state.device, &device_create_info, m_state.allocation_callbacks, &m_state.device.logical_device);
	TK_ASSERT(result == VK_SUCCESS, "Could not create Vulkan device");

	vkGetDeviceQueue(m_state.device, m_state.device.indices[GRAPHICS_FAMILY_INDEX], 0, &m_state.device.graphics_queue);
	vkGetDeviceQueue(m_state.device, m_state.device.indices[PRESENT_FAMILY_INDEX], 0, &m_state.device.present_queue);

	vkDestroySurfaceKHR(m_state.instance, surface, m_state.allocation_callbacks);
}

void VulkanBackend ::initialize_command_pool() {
	VkCommandPoolCreateInfo command_pool_create_info{};
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_create_info.queueFamilyIndex = m_state.device.indices[GRAPHICS_FAMILY_INDEX];

	VkResult result = vkCreateCommandPool(
		m_state.device, &command_pool_create_info, m_state.allocation_callbacks, &m_resources.command_pool);
	TK_ASSERT(result == VK_SUCCESS);

	result = vkCreateCommandPool(
		m_state.device, &command_pool_create_info, m_state.allocation_callbacks, &m_resources.temporary_command_pool);
	TK_ASSERT(result == VK_SUCCESS);
}

void VulkanBackend::initialize_command_buffers() {
	uint32_t count = 1;
	m_resources.command_buffers = allocate_command_buffers(count, m_resources.command_pool);
}

void VulkanBackend::initialize_frames() {
	VkFenceCreateInfo fence_create_info{};
	fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkSemaphoreCreateInfo semaphore_create_info{};
	semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkResult result = VK_SUCCESS;
	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		result = vkCreateFence(
			m_state.device, &fence_create_info, m_state.allocation_callbacks, &m_resources.frames.in_flight_fences[i]);
		TK_ASSERT(result == VK_SUCCESS);

		result = vkCreateSemaphore(
			m_state.device,
			&semaphore_create_info,
			m_state.allocation_callbacks,
			&m_resources.frames.image_available_semaphores[i]);
		TK_ASSERT(result == VK_SUCCESS);

		result = vkCreateSemaphore(
			m_state.device,
			&semaphore_create_info,
			m_state.allocation_callbacks,
			&m_resources.frames.render_finished_semaphores[i]);
		TK_ASSERT(result == VK_SUCCESS);
	}
}

}  // namespace toki::renderer

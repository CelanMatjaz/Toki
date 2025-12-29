#include "toki/renderer/private/vulkan/vulkan_backend.h"

#include <GLFW/glfw3.h>
#include <toki/core/core.h>
#include <toki/renderer/private/vulkan/vulkan_resources_utils.h>
#include <toki/renderer/renderer_allocators.h>
#include <toki/renderer/types.h>

namespace toki {

#define RENDERER (reinterpret_cast<VulkanBackend*>(m_internalData))
#define STATE	 (RENDERER->m_state)

VulkanBackend::VulkanBackend(const RendererConfig& config) {
	initialize(config);
}

VulkanBackend::~VulkanBackend() {
	cleanup();
}

void VulkanBackend::initialize(const RendererConfig& config) {
	TK_LOG_INFO("Initializing Vulkan backend");

	initialize_instance();
	initialize_device(config.window);

	// Important to query device speific data BEFORE creating resources
	vkGetPhysicalDeviceMemoryProperties(m_state.physical_device, &m_state.physical_device_memory_properties);

	m_state.frames = VulkanFrames::create(m_state);

	VulkanSwapchainConfig swapchain_config{};
	swapchain_config.window = config.window;
	m_state.swapchain		= VulkanSwapchain::create(swapchain_config, m_state);
	config.window->register_listener(&m_state.swapchain, VulkanSwapchain::window_listen_function);

	CommandPoolConfig command_pool_config{};
	m_state.command_pool		   = VulkanCommandPool::create(command_pool_config, m_state);
	m_state.temporary_command_pool = VulkanCommandPool::create(command_pool_config, m_state);

	StagingBufferConfig staging_buffer_config{};
	staging_buffer_config.size = toki::MB(500);
	m_state.staging_buffer	   = VulkanStagingBuffer::create(staging_buffer_config, m_state);

	TempDynamicArray<VkDescriptorPoolSize> pool_sizes;
	pool_sizes.emplace_back(VK_DESCRIPTOR_TYPE_SAMPLER, 4);
	pool_sizes.emplace_back(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 4);
	pool_sizes.emplace_back(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4);
	pool_sizes.emplace_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4);

	DescriptorPoolConfig descriptor_pool_config{};
	descriptor_pool_config.max_sets	  = 100;
	descriptor_pool_config.pool_sizes = pool_sizes;
	m_state.descriptor_pool			  = VulkanDescriptorPool::create(descriptor_pool_config, m_state);

	SemaphoreConfig queued_commands_semaphore_config{};
	queued_commands_semaphore_config.stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	m_state.queued_commands_semaphore			 = m_state.semaphores.emplace_at_first<SemaphoreHandle>(
		   VulkanSemaphore::create(queued_commands_semaphore_config, m_state));

	RendererBumpAllocator::reset();
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

void Renderer::frame_prepare() {
	RendererBumpAllocator::reset();

	STATE.frames.frame_prepare(STATE);

	submit([state = &STATE](Commands* cmd) -> void {
		state->swapchain.get_current_image().transition_layout(
			reinterpret_cast<VulkanCommandsData*>(cmd->m_data)->cmd,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	});
}

void Renderer::frame_cleanup() {
	STATE.frames.frame_cleanup(STATE);
}

CommandsHandle Renderer::record_persitent_commands(Function<void(Commands*)> record_function) {
	VulkanCommandBuffer command_buffer = STATE.command_pool.allocate_command_buffers(STATE, 1)[0];
	VulkanCommandsData command_data{};
	command_data.cmd   = command_buffer;
	command_data.state = &STATE;
	Commands commands(&command_data);

	command_buffer.begin();
	record_function(&commands);
	command_buffer.end();

	return { STATE.commands.emplace_at_first(command_buffer) };
}

void Renderer::submit(Function<void(Commands*)> fn) {
	VulkanCommandBuffer command_buffer = STATE.command_pool.allocate_command_buffers(STATE, 1)[0];
	VulkanCommandsData command_data{};
	command_data.cmd   = command_buffer;
	command_data.state = &STATE;
	Commands commands(&command_data);

	command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	fn(&commands);
	command_buffer.end();

	STATE.queued_command_buffers.emplace_back(command_buffer);
}

void Renderer::submit(toki::Span<CommandsHandle> recorded_commands) {
	u32 start = STATE.queued_command_buffers.size();
	STATE.queued_command_buffers.grow(recorded_commands.size());
	for (u32 i = start; i < STATE.queued_command_buffers.size(); i++) {
		TK_ASSERT(STATE.commands.exists(recorded_commands[i]));
		STATE.queued_command_buffers.push_back(STATE.commands.at(recorded_commands[i]));
	}
}

void Renderer::flush_queue(const SubmitOptions& options) {
	TK_ASSERT(STATE.queued_command_buffers.size() > 0);

	TempDynamicArray<VkCommandBuffer> command_buffers(STATE.queued_command_buffers.size());

	for (u32 i = 0; i < STATE.queued_command_buffers.size(); i++) {
		command_buffers[i] = STATE.queued_command_buffers[i].m_commandBuffer;
	}

	TempDynamicArray<VkSemaphore> wait_semaphores(options.wait_semaphores.size());
	TempDynamicArray<VkPipelineStageFlags> wait_stages(options.wait_semaphores.size());
	for (u32 i = 0; i < options.wait_semaphores.size(); i++) {
		TK_ASSERT(STATE.semaphores.exists(options.wait_semaphores[i]))
		VulkanSemaphore& semaphore = STATE.semaphores.at(options.wait_semaphores[i]);
		wait_semaphores[i]		   = semaphore.semaphore();
		wait_stages[i]			   = semaphore.stage_flags();
	}

	TempDynamicArray<VkSemaphore> signal_semaphores(options.signal_semaphores.size());
	for (u32 i = 0; i < options.signal_semaphores.size(); i++) {
		TK_ASSERT(STATE.semaphores.exists(options.signal_semaphores[i]))
		VulkanSemaphore& semaphore = STATE.semaphores.at(options.signal_semaphores[i]);
		signal_semaphores[i]	   = semaphore.semaphore();
	}

	VkSubmitInfo submit_info{};
	submit_info.sType				 = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.pWaitDstStageMask	 = wait_stages.data();
	submit_info.waitSemaphoreCount	 = wait_stages.size();
	submit_info.pWaitSemaphores		 = wait_semaphores.data();
	submit_info.signalSemaphoreCount = signal_semaphores.size();
	submit_info.pSignalSemaphores	 = signal_semaphores.data();
	submit_info.commandBufferCount	 = command_buffers.size();
	submit_info.pCommandBuffers		 = command_buffers.data();

	VkFence fence	= STATE.fences.at(options.signal_fence);
	VkResult result = vkQueueSubmit(STATE.graphics_queue, 1, &submit_info, fence);
	TK_ASSERT(result == VK_SUCCESS);
}

void Renderer::present() {
	submit([state = &STATE](Commands* cmd) -> void {
		state->swapchain.get_current_image().transition_layout(
			reinterpret_cast<VulkanCommandsData*>(cmd->m_data)->cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	});

	if (STATE.queued_command_buffers.size() > 0) {
		SemaphoreHandle wait_semahores[]{ STATE.frames.get_image_available_semaphore_handle() };
		SemaphoreHandle signal_semaphores[]{ STATE.frames.get_render_finished_semaphore_handle() };

		SubmitOptions submit_options{};
		submit_options.wait_semaphores	 = wait_semahores;
		submit_options.signal_semaphores = signal_semaphores;
		submit_options.signal_fence		 = STATE.frames.get_in_flight_fence_handle();
		flush_queue(submit_options);
	}

	VkSemaphore wait_semaphores[] = { STATE.frames.get_render_finished_semaphore(STATE) };
	VkSwapchainKHR swapchains[]	  = { STATE.swapchain.swapchain() };
	Array<u32, 1> image_indices{ STATE.swapchain.image_index() };

	VkPresentInfoKHR present_info{};
	present_info.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores	= wait_semaphores;
	present_info.swapchainCount		= 1;
	present_info.pSwapchains		= swapchains;
	present_info.pImageIndices		= image_indices.data();

	VkResult result = vkQueuePresentKHR(STATE.present_queue, &present_info);
	TK_ASSERT(result == VK_SUCCESS);

	STATE.staging_buffer.reset();
	STATE.queued_command_buffers.clear();
}

void Renderer::set_buffer_data(BufferHandle handle, const void* data, u32 size) {
	TK_ASSERT(STATE.buffers.exists(handle));
	STATE.staging_buffer.set_data_for_buffer(STATE, STATE.buffers.at(handle), data, size);
}

void Renderer::set_texture_data(TextureHandle handle, const void* data, u32 size) {
	TK_ASSERT(STATE.textures.exists(handle));
	STATE.staging_buffer.set_data_for_image(STATE, STATE.textures.at(handle), data, size);
}

void Renderer::set_uniforms(const SetUniformConfig& config) {
	TK_ASSERT(STATE.shader_layouts.exists(config.layout));
	STATE.shader_layouts.at(config.layout).set_descriptors(STATE, config);
}

void VulkanBackend::initialize_instance() {
	VkApplicationInfo application_info{};
	application_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.pApplicationName	= "Toki engine";
	application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	application_info.pEngineName		= "Toki";
	application_info.engineVersion		= VK_MAKE_VERSION(0, 0, 0);
	application_info.apiVersion			= VK_API_VERSION_1_4;

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType			   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo  = &application_info;
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
	// instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	instance_create_info.enabledExtensionCount	 = instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
#else
	instance_create_info.enabledExtensionCount	 = 0;
	instance_create_info.ppEnabledExtensionNames = nullptr;
#endif

#if !defined(TK_DIST)
	// Setup validation layers

	Array<const char*, 1> validation_layers{ "VK_LAYER_KHRONOS_validation" };

	u32 layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	TempDynamicArray<VkLayerProperties> layer_properties(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, layer_properties.data());

	u32 required_layer_count = validation_layers.size();
	for (u32 i = 0; i < validation_layers.size(); ++i) {
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

	instance_create_info.enabledLayerCount	 = validation_layers.size();
	instance_create_info.ppEnabledLayerNames = validation_layers.data();
#else
	instance_create_info.enabledLayerCount	 = 0;
	instance_create_info.ppEnabledLayerNames = nullptr;
#endif

	TK_LOG_INFO("Creating Vulkan instance");
	VkResult result = vkCreateInstance(&instance_create_info, m_state.allocation_callbacks, &m_state.instance);
	TK_ASSERT(result == VK_SUCCESS);
}

void VulkanBackend::initialize_device(Window* window) {
	m_window			 = window;
	VkSurfaceKHR surface = create_surface(m_state, window);

	u32 physical_device_count;
	vkEnumeratePhysicalDevices(m_state.instance, &physical_device_count, nullptr);
	TempDynamicArray<VkPhysicalDevice> physical_devices(physical_device_count);
	vkEnumeratePhysicalDevices(m_state.instance, &physical_device_count, physical_devices.data());

	VkPhysicalDeviceProperties physical_device_properties;
	vkGetPhysicalDeviceProperties(physical_devices[0], &physical_device_properties);

	TK_LOG_INFO("Using GPU: {}, driver: {}", physical_device_properties.deviceName, physical_device_properties.driverVersion);

	m_state.physical_device = physical_devices[0];

	u32 queue_family_count;
	vkGetPhysicalDeviceQueueFamilyProperties(m_state.physical_device, &queue_family_count, nullptr);
	TempDynamicArray<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(
		m_state.physical_device, &queue_family_count, queue_family_properties.data());

	VkBool32 supports_present = false;

	m_state.indices[GRAPHICS_FAMILY_INDEX] = U32_MAX;
	m_state.indices[PRESENT_FAMILY_INDEX]  = U32_MAX;

	for (u32 i = 0; i < queue_family_properties.size(); ++i) {
		if (m_state.indices[PRESENT_FAMILY_INDEX] != U32_MAX && m_state.indices[GRAPHICS_FAMILY_INDEX] != U32_MAX) {
			break;
		}

		if ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
			m_state.indices[PRESENT_FAMILY_INDEX] == U32_MAX) {
			m_state.indices[GRAPHICS_FAMILY_INDEX] = i;
			continue;
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(m_state.physical_device, i, surface, &supports_present);
		if (supports_present && m_state.indices[PRESENT_FAMILY_INDEX] == U32_MAX) {
			m_state.indices[PRESENT_FAMILY_INDEX] = i;
			continue;
		}
	}

	TK_ASSERT(m_state.indices[PRESENT_FAMILY_INDEX] != U32_MAX && m_state.indices[GRAPHICS_FAMILY_INDEX] != U32_MAX);

	f32 queue_priority = 1.0f;

	TempDynamicArray<VkDeviceQueueCreateInfo> queue_create_infos(2);
	for (u32 i = 0; i < queue_create_infos.size(); i++) {
		queue_create_infos[i].sType			   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_infos[i].queueFamilyIndex = m_state.indices[i];
		queue_create_infos[i].queueCount	   = 1;
		queue_create_infos[i].pQueuePriorities = &queue_priority;
	}

	u32 device_extension_count;
	vkEnumerateDeviceExtensionProperties(m_state.physical_device, nullptr, &device_extension_count, nullptr);
	TempDynamicArray<VkExtensionProperties> queried_device_extensions(device_extension_count);
	vkEnumerateDeviceExtensionProperties(
		m_state.physical_device, nullptr, &device_extension_count, queried_device_extensions.data());

	Array<const char*, 1> device_extensions(static_cast<const char*>(VK_KHR_SWAPCHAIN_EXTENSION_NAME));
	// for (u32 i = 0; i < device_extensions.size(); i++) {
	// 	toki::println("{}", device_extensions[i]);
	// }

	VkPhysicalDeviceFeatures physical_device_features{};
	physical_device_features.samplerAnisotropy = VK_TRUE;

	VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features{};
	dynamic_rendering_features.sType			= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamic_rendering_features.dynamicRendering = VK_TRUE;

	VkDeviceCreateInfo device_create_info{};
	device_create_info.sType				   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pNext				   = &dynamic_rendering_features;
	device_create_info.pQueueCreateInfos	   = queue_create_infos.data();
	device_create_info.queueCreateInfoCount	   = queue_create_infos.size();
	device_create_info.pEnabledFeatures		   = &physical_device_features;
	device_create_info.enabledExtensionCount   = device_extensions.size();
	device_create_info.ppEnabledExtensionNames = device_extensions.data();

	TK_LOG_INFO("Creating Vulkan logical device");
	VkResult result = vkCreateDevice(
		m_state.physical_device, &device_create_info, m_state.allocation_callbacks, &m_state.logical_device);
	TK_ASSERT(result == VK_SUCCESS, "Could not create Vulkan device");

	vkDestroySurfaceKHR(m_state.instance, surface, m_state.allocation_callbacks);

	vkGetDeviceQueue(m_state.logical_device, m_state.indices[GRAPHICS_FAMILY_INDEX], 0, &m_state.graphics_queue);
	vkGetDeviceQueue(m_state.logical_device, m_state.indices[PRESENT_FAMILY_INDEX], 0, &m_state.present_queue);
}

#define DEFINE_CREATE_RESOURCE(type, lowercase_type)                                              \
	type##Handle Renderer::create_##lowercase_type(const type##Config& config) {                  \
		return { STATE.lowercase_type##s.emplace_at_first(Vulkan##type::create(config, STATE)) }; \
	}

DEFINE_CREATE_RESOURCE(Shader, shader)
DEFINE_CREATE_RESOURCE(ShaderLayout, shader_layout)
DEFINE_CREATE_RESOURCE(Buffer, buffer)
DEFINE_CREATE_RESOURCE(Texture, texture)
DEFINE_CREATE_RESOURCE(Sampler, sampler)

#undef DEFINE_CREATE_RESOURCE

#define DEFINE_DESTROY_HANDLE(type, arena)       \
	void Renderer::destroy_handle(type handle) { \
		TK_ASSERT(STATE.arena.exists(handle));   \
		STATE.arena.at(handle).destroy(STATE);   \
		STATE.arena.clear(handle);               \
	}

DEFINE_DESTROY_HANDLE(ShaderHandle, shaders);
DEFINE_DESTROY_HANDLE(ShaderLayoutHandle, shader_layouts);
DEFINE_DESTROY_HANDLE(BufferHandle, buffers);
DEFINE_DESTROY_HANDLE(TextureHandle, textures);
DEFINE_DESTROY_HANDLE(SamplerHandle, samplers);

#undef DEFINE_DESTROY_HANDLE

void Renderer::destroy_handle(CommandsHandle handle) {
	TK_ASSERT(STATE.commands.exists(handle));
	STATE.commands.at(handle).free(STATE);
	STATE.commands.clear(handle);
}

}  // namespace toki

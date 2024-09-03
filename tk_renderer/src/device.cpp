#include "device.h"

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "renderer_types.h"
#include "utils/device_utils.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

static const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
static const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

TkError create_instance(VulkanState* state) {
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Toki";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    application_info.pEngineName = "Toki";
    application_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    application_info.apiVersion = VK_API_VERSION_1_3;

    uint32_t extension_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledExtensionCount = extension_count;
    instance_create_info.ppEnabledExtensionNames = extensions;

#ifndef TK_DIST
    bool supported;
    uint32_t layer_count = sizeof(layers) / sizeof(char*);
    ASSERT_ERROR(query_layer_support(layers, layer_count, &supported), "Required instance layers not supported");

    instance_create_info.enabledLayerCount = layer_count;
    instance_create_info.ppEnabledLayerNames = layers;
#endif  // !TK_DIST

    VkResult result = vkCreateInstance(&instance_create_info, state->allocation_callbacks, &state->instance);
    ASSERT_VK_RESULT(result, Error::RENDERER_INIT_ERROR)

    return TkError{};
}

TkError destroy_instance(VulkanState* state) {
    vkDestroyInstance(state->instance, state->allocation_callbacks);
    return TkError{};
}

TkError create_device(VulkanState* state, GLFWwindow* window) {
    RendererWindow renderer_window{};
    renderer_window.window = window;
    ASSERT_ERROR(create_surface(state, window, &renderer_window), "Could not create window surface");

    state->windows.emplace_back(renderer_window);

    ASSERT_ERROR(query_physical_devices(state), "Could not query physical devices");
    ASSERT_ERROR(query_physical_device_properties(&state->physical_device_data), "Could not query physical device properties");
    ASSERT_ERROR(query_physical_device_queues(&state->physical_device_data, renderer_window.surface), "Could not query physical device queues");
    ASSERT_ERROR(query_physical_device_present_modes(&state->physical_device_data, renderer_window.surface), "Could not query present modes");
    ASSERT_ERROR(query_physical_device_surface_formats(&state->physical_device_data, renderer_window.surface), "Could not query surface formats");

    float queue_priority = 1.0f;
    uint32_t queue_families[] = { state->physical_device_data.present_queue_family_index,
                                  state->physical_device_data.graphics_queue_family_index,
                                  state->physical_device_data.transfer_queue_family_index };
    constexpr uint32_t device_queue_info_count = sizeof(queue_families) / sizeof(uint32_t);
    VkDeviceQueueCreateInfo device_queue_create_infos[device_queue_info_count]{};

    for (uint32_t i = 0; i < sizeof(queue_families) / sizeof(uint32_t); ++i) {
        device_queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_infos[i].queueFamilyIndex = queue_families[i];
        device_queue_create_infos[i].queueCount = 1;
        device_queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    VkPhysicalDeviceFeatures physical_device_features{};

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pEnabledFeatures = &physical_device_features;
    device_create_info.pQueueCreateInfos = device_queue_create_infos;
    device_create_info.queueCreateInfoCount = device_queue_info_count;
    device_create_info.enabledExtensionCount = sizeof(extensions) / sizeof(const char*);
    device_create_info.ppEnabledExtensionNames = extensions;

    VkResult result = vkCreateDevice(state->physical_device_data.physical_device, &device_create_info, state->allocation_callbacks, &state->device);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_DEVICE_ERROR);

    // vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue)

    return TkError{};
}

TkError destroy_device(VulkanState* state) {
    vkDestroyDevice(state->device, state->allocation_callbacks);
    return TkError{};
}

}  // namespace Toki

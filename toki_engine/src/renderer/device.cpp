#include "device.h"

#include <vulkan/vulkan.h>

#include "core/core.h"
#include "core/logging.h"
#include "macros.h"
#include "renderer/platform/renderer_utils.h"
#include "utils.h"

namespace toki {

TkError create_instance(RendererContext* ctx) {
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Toki";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    application_info.pEngineName = "Toki Engine";
    application_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    uint32_t extensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledExtensionCount = extensionCount;
    instance_create_info.ppEnabledExtensionNames = extensions;

#ifndef TK_DIST
    TK_ASSERT(check_validation_layer_support(), "Validation layers not supported");
    instance_create_info.enabledLayerCount = validation_layers.size();
    instance_create_info.ppEnabledLayerNames = validation_layers.data();
#endif

    TK_ASSERT_VK_RESULT(
        vkCreateInstance(&instance_create_info, ctx->allocationCallbacks, &ctx->instance),
        "Could not initialize renderer");

    return {};
}

TkError create_device(RendererContext* ctx, std::shared_ptr<Window> window) {
    Scope<VkSurfaceKHR, VK_NULL_HANDLE> temp_surface(VK_NULL_HANDLE, [&](VkSurfaceKHR s) {
        vkDestroySurfaceKHR(ctx->instance, s, ctx->allocationCallbacks);
    });
    create_surface(ctx, (GLFWwindow*) window->get_handle(), &temp_surface.ref());

    u32 physical_device_count{};
    vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, nullptr);
    TK_ASSERT(physical_device_count > 0, "No GPUs found");
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(ctx->instance, &physical_device_count, physical_devices.data());

    u32 device_index = 0;
    for (u32 i = 0; i < physical_devices.size(); i++) {
        bool is_suitable = is_device_suitable(physical_devices[i]);
        if (is_suitable) {
            device_index = i;
            break;
        } else if (i == physical_devices.size() - 1) {
            TK_ASSERT(false, "No suitable GPU found");
        }
    }

    VkPhysicalDevice physical_device = physical_devices[device_index];

    const auto [graphics, present, transfer] = find_queue_families(physical_device, temp_surface);

    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_infos[2]{};
    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = graphics;
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queue_priority;
    queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[1].queueFamilyIndex = present;
    queue_create_infos[1].queueCount = 1;
    queue_create_infos[1].pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures features{};

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.queueCreateInfoCount = graphics == present ? 1 : 2;
    device_create_info.pEnabledFeatures = &features;

    TK_ASSERT_VK_RESULT(
        vkCreateDevice(
            physical_device, &device_create_info, ctx->allocationCallbacks, &ctx->device),
        "Could not create device");

    return {};
}

}  // namespace toki

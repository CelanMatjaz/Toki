#include "device.h"

#include <GLFW/glfw3.h>
#include <toki/core.h>

#include <utility>

#include "renderer_types.h"
#include "utils/device_utils.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

void renderer_create_instance() {
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
    if (renderer_utils_check_for_validation_layer_support()) {
        instance_create_info.enabledLayerCount = validation_layers.size();
        instance_create_info.ppEnabledLayerNames = validation_layers.data();
    } else {
        TK_ASSERT(false, "1 or more validation layers not supported by the system");
    }
#endif  // !DIST

    TK_ASSERT_VK_RESULT(vkCreateInstance(&instance_create_info, s_context.allocation_callbacks, &s_context.instance), "Could not create instance");
}

void destroy_instance() {
    vkDestroyInstance(s_context.instance, s_context.allocation_callbacks);
}

void create_device(const PhysicalDevice& pd, const CreateDeviceConfig& config) {
    PhysicalDeviceData* physical_device_data = ((PhysicalDeviceData*) pd.internal_data);
    VkPhysicalDevice physical_device = physical_device_data->physical_device;

    float queuePriority = 1.0f;

    DeviceQueues device_queues;
}

void destroy_device() {}

std::vector<PhysicalDevice> enumerate_physical_devices() {
    uint32_t count{};
    vkEnumeratePhysicalDevices(s_context.instance, &count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(count);
    vkEnumeratePhysicalDevices(s_context.instance, &count, physical_devices.data());

    std::vector<PhysicalDevice> devices(count);

    for (uint32_t i = 0; i < count; ++i) {
        const auto& device = physical_devices[i];
        auto enumerated_device = *((PhysicalDeviceData*) devices[i].internal_data);

        vkGetPhysicalDeviceProperties(device, &enumerated_device.device_properties);
        vkGetPhysicalDeviceFeatures(device, &enumerated_device.device_features);
        vkGetPhysicalDeviceMemoryProperties(device, &enumerated_device.memory_properties);
    }

    return devices;
}

const char* get_physical_device_name(const PhysicalDevice& physical_device) {
    return ((PhysicalDeviceData*) physical_device.internal_data)->device_properties.deviceName;
}

uint32_t get_physical_device_id(const PhysicalDevice& physical_device) {
    return ((PhysicalDeviceData*) physical_device.internal_data)->device_properties.deviceID;
}

uint32_t get_physical_device_vendor_id(const PhysicalDevice& physical_device) {
    return ((PhysicalDeviceData*) physical_device.internal_data)->device_properties.vendorID;
}

uint32_t get_physical_device_api_version(const PhysicalDevice& physical_device) {
    return ((PhysicalDeviceData*) physical_device.internal_data)->device_properties.apiVersion;
}

uint32_t get_physical_device_driver_version(const PhysicalDevice& physical_device) {
    return ((PhysicalDeviceData*) physical_device.internal_data)->device_properties.driverVersion;
}

PhysicalDeviceType get_physical_device_type(const PhysicalDevice& physical_device) {
    switch (((PhysicalDeviceData*) physical_device.internal_data)->device_properties.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            return PhysicalDeviceType::Other;
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return PhysicalDeviceType::IntegratedGPU;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return PhysicalDeviceType::DiscreteGPU;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return PhysicalDeviceType::VirtualGPU;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return PhysicalDeviceType::CPU;
        default:
            std::unreachable();
    }
}

}  // namespace Toki

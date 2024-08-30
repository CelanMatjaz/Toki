#include "device.h"

#include <GLFW/glfw3.h>
#include <toki/core.h>

#include <utility>

#include "renderer_types.h"
#include "vulkan/vulkan_core.h"

namespace Toki {

void renderer_create_device(const PhysicalDevice& pd, const CreateDeviceConfig& config){
    PhysicalDeviceData* physical_device_data = ((PhysicalDeviceData*) pd.internal_data);
    VkPhysicalDevice physical_device = physical_device_data->physical_device;

    float queuePriority = 1.0f;

    DeviceQueues device_queues;
}

void renderer_destroy_device() {}

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

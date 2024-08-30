#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace Toki {

struct PhysicalDevice {
    void* internal_data;
};

struct CreateDeviceConfig {};

void renderer_create_device(const PhysicalDevice& physical_device, const CreateDeviceConfig& config);
void renderer_destroy_device();

enum class PhysicalDeviceType {
    Other,
    IntegratedGPU,
    DiscreteGPU,
    VirtualGPU,
    CPU
};

std::vector<PhysicalDevice> enumerate_physical_devices();

const char* get_physical_device_name(const PhysicalDevice& physical_device);
uint32_t get_physical_device_id(const PhysicalDevice& physical_device);
uint32_t get_physical_device_vendor_id(const PhysicalDevice& physical_device);
uint32_t get_physical_device_api_version(const PhysicalDevice& physical_device);
uint32_t get_physical_device_driver_version(const PhysicalDevice& physical_device);
PhysicalDeviceType get_physical_device_type(const PhysicalDevice& physical_device);

}  // namespace Toki

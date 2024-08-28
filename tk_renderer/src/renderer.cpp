#include "renderer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <print>
#include <vector>

namespace Renderer {

VkInstance instance;

void render() {
    VkInstanceCreateInfo info{};
    VkResult res = vkCreateInstance(&info, nullptr, &instance);

    uint32_t groupCount{};
    vkEnumeratePhysicalDevices(instance, &groupCount, nullptr);
    std::vector<VkPhysicalDevice> devices(groupCount);
    vkEnumeratePhysicalDevices(instance, &groupCount, devices.data());

    std::println("Test123 {} {}", (int) res, groupCount);

    for(const auto& d : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(d, &props);
        std::println("{}", props.deviceName);
    }
}

}  // namespace Renderer

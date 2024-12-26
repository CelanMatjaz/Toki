#include "device.h"

#include <string.h>

#include "GLFW/glfw3native.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>

#include <array>

#include "macros.h"

namespace toki {

#ifndef TK_DIST
static std::array<const char*, 1> validation_layers = {
    "VK_LAYER_KHRONOS_validation",
};
#endif

TkError create_instance(RendererState* state) {
    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "Toki";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "Toki";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    uint32_t extensionCount = 0;
    const char** extensions =
        glfwGetRequiredInstanceExtensions(&extensionCount);

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;
    instance_create_info.enabledExtensionCount = extensionCount;
    instance_create_info.ppEnabledExtensionNames = extensions;

#ifndef TK_DIST
    if (check_validation_layel_support()) {
        instance_create_info.enabledLayerCount = validation_layers.size();
        instance_create_info.ppEnabledLayerNames = validation_layers.data();
    }
#endif

    TK_ASSERT_VK_RESULT(
        vkCreateInstance(
            &instance_create_info,
            state->allocation_callbacks,
            &state->instance),
        "Could not initialize renderer");

    return {};
}

TkError create_device(RendererState* state) {
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    return {};
}

bool check_validation_layel_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    for (const char* required_layer : validation_layers) {
        bool layer_found = false;

        for (const auto& found_layer : layers) {
            if (strcmp(required_layer, found_layer.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) return false;
    }

    return true;
}

}  // namespace toki

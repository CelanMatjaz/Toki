#include "renderer.h"

#include <toki/core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "utils/device_utils.h"

namespace Toki {

struct RendererState {
} static* s_renderer_state = nullptr;

void renderer_create_instance();
void renderer_destroy_instance();

void renderer_initialize_state(const RendererStateConfig& config) {
    TK_ASSERT(s_renderer_state == nullptr, "Renderer state is already initialized");
    s_renderer_state = new RendererState();
    renderer_create_instance();
}

void renderer_destroy_state() {
    TK_ASSERT(s_renderer_state != nullptr, "Renderer state is not initialized");
    renderer_destroy_instance();
    delete s_renderer_state;
    s_renderer_state = nullptr;
}

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

void renderer_destroy_instance() {
    vkDestroyInstance(s_context.instance, s_context.allocation_callbacks);
}

}  // namespace Toki

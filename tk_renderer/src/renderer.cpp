#include "renderer.h"

#include <toki/core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "renderer_types.h"

namespace Toki {

void create_instance();
void destroy_instance();

void renderer_initialize_state() {
    TK_ASSERT(s_renderer_state == nullptr, "Renderer state is already initialized");
    s_renderer_state = new VulkanState();
    create_instance();
}

void renderer_destroy_state() {
    TK_ASSERT(s_renderer_state != nullptr, "Renderer state is not initialized");
    destroy_instance();
    delete s_renderer_state;
    s_renderer_state = nullptr;
}

void renderer_initialize(const RendererInitConfig& config) {}

void renderer_shutdown() {}

void create_instance() {
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

    TK_ASSERT_VK_RESULT(
        vkCreateInstance(&instance_create_info, s_renderer_state->allocation_callbacks, &s_renderer_state->instance), "Could not create instance");
}

void destroy_instance() {
    vkDestroyInstance(s_renderer_state->instance, s_renderer_state->allocation_callbacks);
}

}  // namespace Toki

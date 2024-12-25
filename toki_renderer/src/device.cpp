#include "device.h"

#include "vulkan/vulkan.h"

namespace toki {

TkError create_instance(Renderer::RendererState* state) {
    VkApplicationInfo application_info{};

    VkInstanceCreateInfo instance_create_info{};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &application_info;

    vkCreateInstance(
        &instance_create_info, state->allocation_callbacks, &state->instance);

    return {};
}

TkError create_device(Renderer::RendererState* state) {}

}  // namespace toki

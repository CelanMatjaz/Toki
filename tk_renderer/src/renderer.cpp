#include "renderer.h"

#include <toki/core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "renderer_types.h"
#include "utils/device_utils.h"

namespace Toki {

static VulkanState s_renderer_state{};

TkError renderer_initialize(const RendererInitConfig& config) {
    ASSERT_ERROR(create_instance(&s_renderer_state), "Error creating Vulkan instance");
    ASSERT_ERROR(create_device(&s_renderer_state, config.initial_window), "Error creating Vulkan device");
    return TkError{};
}

TkError renderer_shutdown() {
    for (uint32_t i = 0; i < s_renderer_state.windows.size(); ++i) {
        destroy_renderer_window(&s_renderer_state, &s_renderer_state.windows[i]);
    }

    ASSERT_ERROR(destroy_device(&s_renderer_state), "Error destroyign Vulkan device");
    ASSERT_ERROR(destroy_instance(&s_renderer_state), "Error destroying Vulkan instance");
    return TkError{};
}

}  // namespace Toki

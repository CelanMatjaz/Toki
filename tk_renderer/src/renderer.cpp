#include "renderer.h"

#include <toki/core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "device.h"
#include "renderer_types.h"
#include "utils/device_utils.h"

namespace Toki {

static VulkanState renderer_state{};
static RendererFrame frames[MAX_FRAMES];

TkError renderer_initialize(const RendererInitConfig& config) {
    ASSERT_ERROR(create_instance(&renderer_state), "Error creating Vulkan instance");
    ASSERT_ERROR(create_device(&renderer_state, config.initial_window), "Error creating Vulkan device");
    return TkError{};
}

TkError renderer_shutdown() {
    for (uint32_t i = 0; i < renderer_state.windows.size(); ++i) {
        destroy_renderer_window(&renderer_state, &renderer_state.windows[i]);
    }

    destroy_device(&renderer_state);
    destroy_instance(&renderer_state);
    return TkError{};
}

TkError renderer_begin_frame(FrameData& frame_data) {
    return TkError{};
}

TkError renderer_end_frame(FrameData& frame_data) {
    return TkError{};
}

TkError renderer_present(FrameData& frame_data) {
    return TkError{};
}

}  // namespace Toki

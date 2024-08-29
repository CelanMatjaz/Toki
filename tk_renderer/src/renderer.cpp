#include "renderer.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "device.h"

namespace Toki {

void renderer_initialize(GLFWwindow* initial_window) {
    renderer_create_instance();
}

void renderer_shutdown() {}

}  // namespace Tok

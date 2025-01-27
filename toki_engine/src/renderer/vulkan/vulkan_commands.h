#pragma once

#include "renderer/vulkan/vulkan_backend.h"

namespace toki {

namespace vulkan_renderer {

class VulkanCommands {
    VulkanCommands() = delete;
    VulkanCommands(VulkanBackend* backend): m_backend(backend) {}

public:
    void draw(u32 count);
    void draw_indexed(u32 count);

private:
    VulkanBackend* m_backend;
};

}  // namespace vulkan_renderer

}  // namespace toki

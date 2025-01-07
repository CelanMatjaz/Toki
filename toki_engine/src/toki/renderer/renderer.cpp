#include "renderer.h"

#include "renderer/vulkan/vulkan_renderer.h"

namespace toki {

std::shared_ptr<renderer> renderer::create(const Config& config) {
    return std::make_shared<vulkan_renderer>(config);
}

renderer::renderer(const Config& config) {}

std::shared_ptr<renderer_api> renderer::get_renderer_api() const {
    return _renderer_api;
}

}  // namespace toki

#pragma once

#include "renderer/vulkan/renderer_state.h"

namespace toki {

struct ImageViewConfig {
    VkImage image{};
    VkImageViewType viewType{};
    VkFormat format{};
};

VkImageView create_image_view(const RendererContext* state, const ImageViewConfig& config);

}  // namespace toki

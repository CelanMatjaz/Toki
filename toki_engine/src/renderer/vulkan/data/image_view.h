#pragma once

#include "renderer/vulkan/renderer_state.h"

namespace toki {

struct ImageView {
    struct Config {
        VkImage image{};
        VkImageViewType viewType{};
        VkFormat format{};
    };

    static ImageView create(Ref<RendererContext> ctx, const Config& config);
    static void cleanup(Ref<RendererContext> ctx, ImageView& image_view);

    VkImageView imageView{};
};

}  // namespace toki

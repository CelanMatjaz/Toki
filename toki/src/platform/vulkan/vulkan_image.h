#pragma once

#include "vulkan/vulkan_types.h"

namespace Toki {

class VulkanImage {
public:
    VulkanImage() = delete;
    VulkanImage(const VulkanContext* context);
    ~VulkanImage();

    static VkImageView createImageView(const VulkanContext* context, const ImageViewConfig& imageViewConfig);

private:
};

}  // namespace Toki

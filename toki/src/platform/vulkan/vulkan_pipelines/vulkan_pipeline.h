#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

class VulkanPipeline {
public:
    VulkanPipeline(const VulkanContext* context);
    virtual ~VulkanPipeline();

    VkPipeline getHandle();
    void recreate();

protected:
    virtual void create() = 0;
    virtual void destroy() = 0;

    const VulkanContext* context;
    VkPipeline pipeline = VK_NULL_HANDLE;
};

}  // namespace Toki

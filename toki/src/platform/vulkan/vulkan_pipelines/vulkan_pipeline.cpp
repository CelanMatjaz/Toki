#include "vulkan_pipeline.h"

namespace Toki {

VulkanPipeline::VulkanPipeline(const VulkanContext* context) : context(context) {}

VulkanPipeline::~VulkanPipeline() {}

VkPipeline VulkanPipeline::getHandle() {
    return pipeline;
}

void VulkanPipeline::recreate() {
    destroy();
    create();
}

}  // namespace Toki

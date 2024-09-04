#include "descriptors.h"

#include "vulkan/vulkan_core.h"

namespace Toki {

static constexpr uint32_t MAX_DESCRIPTOR_SETS = 100;

TkError create_descriptor_pool(VulkanState* state) {
    static VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
                                                 { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
                                                 { VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
                                                 { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 } };
    uint32_t pool_size_count = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);

    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.maxSets = MAX_DESCRIPTOR_SETS;
    descriptor_pool_create_info.poolSizeCount = pool_size_count;
    descriptor_pool_create_info.pPoolSizes = pool_sizes;

    VkResult result = vkCreateDescriptorPool(state->device, &descriptor_pool_create_info, state->allocation_callbacks, &state->descriptor_pool);

    return TkError{};
}

void destroy_descriptor_pool(VulkanState* state) {
}

}  // namespace Toki

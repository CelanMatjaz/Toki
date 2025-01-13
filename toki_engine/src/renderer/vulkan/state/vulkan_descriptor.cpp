#include "vulkan_descriptor.h"

#include "core/assert.h"
#include "renderer/vulkan/vulkan_context.h"

namespace toki {

void DescriptorPoolManager::create(Ref<RendererContext> ctx, u32 max_set_count, std::span<VkDescriptorPoolSize> pool_sizes) {
    m_pool = create_pool(ctx, max_set_count, pool_sizes);
}

void DescriptorPoolManager::destroy(Ref<RendererContext> ctx) {
    vkDestroyDescriptorPool(ctx->device, m_pool, ctx->allocation_callbacks);
}

void DescriptorPoolManager::clear(Ref<RendererContext> ctx) {
    vkResetDescriptorPool(ctx->device, m_pool, 0);
}

VkDescriptorSet DescriptorPoolManager::allocate_single(Ref<RendererContext> ctx, VkDescriptorSetLayout layout) {
    std::vector<VkDescriptorSetLayout> layouts{ layout };
    return allocate_multiple(ctx, layouts).front();
}

std::vector<VkDescriptorSet> DescriptorPoolManager::allocate_multiple(Ref<RendererContext> ctx, std::span<VkDescriptorSetLayout> layouts) {
    VkDescriptorSetAllocateInfo descriptor_set_allocate_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    descriptor_set_allocate_info.descriptorPool = m_pool;
    descriptor_set_allocate_info.descriptorSetCount = layouts.size();
    descriptor_set_allocate_info.pSetLayouts = layouts.data();

    std::vector<VkDescriptorSet> descriptor_sets(layouts.size());
    VK_CHECK(vkAllocateDescriptorSets(ctx->device, &descriptor_set_allocate_info, descriptor_sets.data()), "Could not allocate descriptor set");

    return descriptor_sets;
}

VkDescriptorPool DescriptorPoolManager::create_pool(Ref<RendererContext> ctx, u32 max_set_count, std::span<VkDescriptorPoolSize> pool_sizes) {
    VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
    descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_create_info.poolSizeCount = pool_sizes.size();
    descriptor_pool_create_info.pPoolSizes = pool_sizes.data();
    descriptor_pool_create_info.maxSets = max_set_count;

    VkDescriptorPool pool;
    VK_CHECK(vkCreateDescriptorPool(ctx->device, &descriptor_pool_create_info, ctx->allocation_callbacks, &pool), "Could not create descriptor pools")
    return pool;
}

void DescriptorWriter::write_buffer(u32 binding, VkDescriptorType type, const WriteBufferConfig& config) {
    VkDescriptorBufferInfo& descriptor_buffer_info = m_bufferInfos.emplace_back(config.buffer, config.offset, config.size);

    VkWriteDescriptorSet write_descriptor_set{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write_descriptor_set.dstBinding = binding;
    write_descriptor_set.pBufferInfo = &descriptor_buffer_info;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = type;

    m_writes.emplace_back(write_descriptor_set);
}

void DescriptorWriter::write_image(u32 binding, VkDescriptorType type, const WriteImageConfig& config) {
    VkDescriptorImageInfo& descriptor_image_info = m_imageInfos.emplace_back(config.sampler, config.image_view, config.image_layout);

    VkWriteDescriptorSet write_descriptor_set{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write_descriptor_set.dstBinding = binding;
    write_descriptor_set.pImageInfo = &descriptor_image_info;
    write_descriptor_set.descriptorCount = 1;
    write_descriptor_set.descriptorType = type;

    m_writes.emplace_back(write_descriptor_set);
}

void DescriptorWriter::clear() {
    m_writes.clear();
    m_bufferInfos.clear();
}

void DescriptorWriter::update_set(Ref<RendererContext> ctx, VkDescriptorSet set) {
    if (m_writes.size() == 0) {
        return;
    }

    for (auto& write : m_writes) {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(ctx->device, m_writes.size(), m_writes.data(), 0, nullptr);
}

}  // namespace toki

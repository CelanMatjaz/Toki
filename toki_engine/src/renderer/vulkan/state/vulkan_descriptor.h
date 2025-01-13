#pragma once

#include <vulkan/vulkan.h>

#include "core/base.h"

namespace toki {

struct RendererContext;

class DescriptorPoolManager {
public:
    DescriptorPoolManager() = default;
    ~DescriptorPoolManager() = default;

    void create(Ref<RendererContext> ctx, u32 max_set_count, std::span<VkDescriptorPoolSize> pool_sizes);
    void destroy(Ref<RendererContext> ctx);

    void clear(Ref<RendererContext> ctx);
    VkDescriptorSet allocate_single(Ref<RendererContext> ctx, VkDescriptorSetLayout layout);
    std::vector<VkDescriptorSet> allocate_multiple(Ref<RendererContext> ctx, std::span<VkDescriptorSetLayout> layouts);

private:
    VkDescriptorPool m_pool;

    static VkDescriptorPool create_pool(Ref<RendererContext> ctx, u32 max_set_count, std::span<VkDescriptorPoolSize> pool_sizes);
};

class DescriptorWriter {
public:
    struct WriteBufferConfig {
        VkBuffer buffer;
        u32 size;
        u32 offset;
    };

    struct WriteImageConfig {
        VkSampler sampler;
        VkImageView image_view;
        VkImageLayout image_layout;
    };

public:
    void write_buffer(u32 binding, VkDescriptorType type, const WriteBufferConfig& config);
    void write_image(u32 binding, VkDescriptorType type, const WriteImageConfig& config);

    void clear();
    void update_set(Ref<RendererContext> ctx, VkDescriptorSet set);

private:
    std::vector<VkDescriptorBufferInfo> m_bufferInfos;
    std::vector<VkDescriptorImageInfo> m_imageInfos;
    std::vector<VkWriteDescriptorSet> m_writes;
};

}  // namespace toki

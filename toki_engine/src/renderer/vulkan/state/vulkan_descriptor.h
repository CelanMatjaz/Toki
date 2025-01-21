#pragma once

#include <vulkan/vulkan.h>

#include <span>

#include "core/base.h"

namespace toki {

struct RendererContext;

class DescriptorPoolManager {
public:
    DescriptorPoolManager() = default;
    ~DescriptorPoolManager() = default;

    void create(RendererContext* ctx, u32 max_set_count, std::span<VkDescriptorPoolSize> pool_sizes);
    void destroy(RendererContext* ctx);

    void clear(RendererContext* ctx);
    VkDescriptorSet allocate_single(RendererContext* ctx, VkDescriptorSetLayout layout);
    std::vector<VkDescriptorSet> allocate_multiple(RendererContext* ctx, std::span<VkDescriptorSetLayout> layouts);

private:
    VkDescriptorPool m_pool;

    static VkDescriptorPool create_pool(RendererContext* ctx, u32 max_set_count, std::span<VkDescriptorPoolSize> pool_sizes);
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
    void update_set(RendererContext* ctx, VkDescriptorSet set);

private:
    std::vector<VkDescriptorBufferInfo> m_bufferInfos;
    std::vector<VkDescriptorImageInfo> m_imageInfos;
    std::vector<VkWriteDescriptorSet> m_writes;
};

}  // namespace toki

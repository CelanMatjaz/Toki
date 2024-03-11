#pragma once

#include <vulkan/vulkan.h>

#include "renderer/vulkan_types.h"
#include "toki/renderer/buffer.h"

namespace Toki {

class VulkanRenderer;

class VulkanBuffer {
    friend VulkanRenderer;

public:
    VulkanBuffer() = delete;

    VulkanBuffer(uint32_t size, VkMemoryPropertyFlags usage, VkMemoryPropertyFlags memoryPropertyFlags);
    VulkanBuffer(uint32_t size, BufferType bufferType);
    virtual ~VulkanBuffer();

    void setData(uint32_t size, void* data, uint32_t offset = 0);
    void* mapMemory(uint32_t size, uint32_t offset);
    void unmapMemory();

    VkBuffer getHandle() const;

private:
    inline static Ref<VulkanContext> s_context;

    VkBuffer m_bufferHandle = VK_NULL_HANDLE;
    VkDeviceMemory m_memoryHandle = VK_NULL_HANDLE;
    uint32_t m_size = 0;
    bool m_isMemoryMapped : 1 = false;
};

class VulkanVertexBuffer : public VertexBuffer, public VulkanBuffer {
public:
    VulkanVertexBuffer() = delete;
    VulkanVertexBuffer(const VertexBufferConfig& config);
    virtual ~VulkanVertexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) override;
    virtual void* mapMemory(uint32_t size, uint32_t offset) override;
    virtual void unmapMemory() override;
};

class VulkanIndexBuffer : public IndexBuffer, public VulkanBuffer {
public:
    VulkanIndexBuffer() = delete;
    VulkanIndexBuffer(const IndexBufferConfig& config);
    virtual ~VulkanIndexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) override;
    virtual void* mapMemory(uint32_t size, uint32_t offset) override;
    virtual void unmapMemory() override;
};

class VulkanUniformBuffer : public UniformBuffer, public VulkanBuffer {
public:
    VulkanUniformBuffer() = delete;
    VulkanUniformBuffer(const UniformBufferConfig& config);
    virtual ~VulkanUniformBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) override;
    virtual void* mapMemory(uint32_t size, uint32_t offset) override;
    virtual void unmapMemory() override;
};

}  // namespace Toki

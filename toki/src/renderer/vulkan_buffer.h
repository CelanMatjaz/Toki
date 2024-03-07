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

    VulkanBuffer(uint32_t size, BufferType bufferType);
    virtual ~VulkanBuffer();

    void setData(uint32_t size, void* data, uint32_t offset = 0);

    VkBuffer getHandle() const;

private:
    inline static Ref<VulkanContext> s_context;

    VkBuffer m_bufferHandle = VK_NULL_HANDLE;
    VkDeviceMemory m_memoryHandle = VK_NULL_HANDLE;
    uint32_t m_size = 0;
};

class VulkanVertexBuffer : public VertexBuffer, public VulkanBuffer {
public:
    VulkanVertexBuffer() = delete;
    VulkanVertexBuffer(const VertexBufferConfig& config);
    virtual ~VulkanVertexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) override;
};

class VulkanIndexBuffer : public IndexBuffer, public VulkanBuffer {
public:
    VulkanIndexBuffer() = delete;
    VulkanIndexBuffer(const IndexBufferConfig& config);
    virtual ~VulkanIndexBuffer() = default;

    virtual void setData(uint32_t size, void* data, uint32_t offset = 0) override;
};

}  // namespace Toki

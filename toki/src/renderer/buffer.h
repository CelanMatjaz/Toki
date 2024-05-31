#pragma once

#include <vulkan/vulkan.h>

#include "toki/resources/configs.h"

namespace Toki {

struct Buffer {
    Buffer() = delete;
    Buffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType type = BufferType::None);
    ~Buffer();

    void setData(uint32_t size, uint32_t offset, void* data);

    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    uint32_t m_size;
    BufferType m_type = BufferType::None;
};

}  // namespace Toki

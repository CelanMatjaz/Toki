#include "buffer.h"

#include "renderer/mapping_functions.h"
#include "renderer/vulkan_types.h"

namespace Toki {

Buffer::Buffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, BufferType type) : m_size(size), m_type(type) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.usage = usage;

    TK_ASSERT_VK_RESULT(vkCreateBuffer(context.device, &bufferCreateInfo, context.allocationCallbacks, &m_buffer), "Could not create buffer");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(context.device, m_buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    TK_ASSERT_VK_RESULT(
        vkAllocateMemory(context.device, &memoryAllocateInfo, context.allocationCallbacks, &m_memory), "Could not allocate buffer memory");
    TK_ASSERT_VK_RESULT(vkBindBufferMemory(context.device, m_buffer, m_memory, 0), "Could not bind buffer memory");
}

Buffer::~Buffer() {
    if (m_memory != VK_NULL_HANDLE) {
        vkFreeMemory(context.device, m_memory, context.allocationCallbacks);
    }

    if (m_buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(context.device, m_buffer, context.allocationCallbacks);
    }
}

void Buffer::setData(uint32_t size, uint32_t offset, void* data) {
    TK_ASSERT(size + offset <= m_size, "Cannot write buffer data out of bounds {} (size) + {} (offset) > {} (buffer size)", size, offset, m_size);
    void* deviceData;
    vkMapMemory(context.device, m_memory, offset, size, 0, &deviceData);
    memcpy(deviceData, data, size);
    vkUnmapMemory(context.device, m_memory);
}

}  // namespace Toki

#include "vulkan_buffer.h"

#include <toki/core/assert.h>

#include "renderer/vulkan_utils.h"

namespace Toki {

VulkanBuffer::VulkanBuffer(uint32_t size, VkMemoryPropertyFlags usage, VkMemoryPropertyFlags memoryPropertyFlags) : m_size(size) {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.usage = usage;

    TK_ASSERT_VK_RESULT(
        vkCreateBuffer(s_context->device, &bufferCreateInfo, s_context->allocationCallbacks, &m_bufferHandle), "Could not create buffer");

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(s_context->device, m_bufferHandle, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex =
        VulkanUtils::findMemoryType(s_context->physicalDevice, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

    TK_ASSERT_VK_RESULT(
        vkAllocateMemory(s_context->device, &memoryAllocateInfo, s_context->allocationCallbacks, &m_memoryHandle),
        "Could not allocate buffer memory");
    TK_ASSERT_VK_RESULT(vkBindBufferMemory(s_context->device, m_bufferHandle, m_memoryHandle, 0), "Could not bind buffer memory");
}

static VkBufferUsageFlags mapUsage(BufferType bufferType) {
    switch (bufferType) {
        case BufferType::BUFFER_TYPE_VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferType::BUFFER_TYPE_INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        case BufferType::BUFFER_TYPE_UNIFORM:
            return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        default:
            std::unreachable();
    }
}

VulkanBuffer::VulkanBuffer(uint32_t size, BufferType bufferType) :
    VulkanBuffer(size, mapUsage(bufferType), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {}

VulkanBuffer::~VulkanBuffer() {
    if (m_isMemoryMapped) {
        unmapMemory();
    }

    vkDestroyBuffer(s_context->device, m_bufferHandle, s_context->allocationCallbacks);
    vkFreeMemory(s_context->device, m_memoryHandle, s_context->allocationCallbacks);
}

void VulkanBuffer::setData(uint32_t size, void* data, uint32_t offset) {
    void* deviceData;
    vkMapMemory(s_context->device, m_memoryHandle, offset, size, 0, &deviceData);
    memcpy(deviceData, data, size);
    vkUnmapMemory(s_context->device, m_memoryHandle);
}

void* VulkanBuffer::mapMemory(uint32_t size, uint32_t offset) {
    TK_ASSERT(offset + size <= m_size, "Trying to map over buffer size");
    void* deviceData = nullptr;
    TK_ASSERT_VK_RESULT(vkMapMemory(s_context->device, m_memoryHandle, offset, size, 0, &deviceData), "Could not map memory");
    m_isMemoryMapped = true;
    return deviceData;
}
void VulkanBuffer::unmapMemory() {
    vkUnmapMemory(s_context->device, m_memoryHandle);
    m_isMemoryMapped = false;
}

VkBuffer VulkanBuffer::getHandle() const {
    return m_bufferHandle;
}

VulkanVertexBuffer::VulkanVertexBuffer(const VertexBufferConfig& config) :
    VertexBuffer(config),
    VulkanBuffer(config.size, BufferType::BUFFER_TYPE_VERTEX) {}

void VulkanVertexBuffer::setData(uint32_t size, void* data, uint32_t offset) {
    VulkanBuffer::setData(size, data, offset);
}

VulkanIndexBuffer::VulkanIndexBuffer(const IndexBufferConfig& config) :
    IndexBuffer(config),
    VulkanBuffer(config.size, BufferType::BUFFER_TYPE_INDEX) {}

void VulkanIndexBuffer::setData(uint32_t size, void* data, uint32_t offset) {
    VulkanBuffer::setData(size, data, offset);
}

VulkanUniformBuffer::VulkanUniformBuffer(const UniformBufferConfig& config) :
    UniformBuffer(config),
    VulkanBuffer(config.size, BufferType::BUFFER_TYPE_UNIFORM) {}

void VulkanUniformBuffer::setData(uint32_t size, void* data, uint32_t offset) {
    VulkanBuffer::setData(size, data, offset);
}

void* VulkanUniformBuffer::mapMemory(uint32_t size, uint32_t offset) {
    return VulkanBuffer::mapMemory(size, offset);
}

void VulkanUniformBuffer::unmapMemory() {
    VulkanBuffer::unmapMemory();
}

}  // namespace Toki

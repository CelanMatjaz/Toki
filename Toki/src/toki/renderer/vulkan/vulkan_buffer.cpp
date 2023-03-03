#include "vulkan_buffer.h"

namespace Toki {

    VulkanBuffer::VulkanBuffer(uint32_t size) : size{ size } {}

    void VulkanBuffer::setData(uint32_t size, void* dataIn) {
        TK_ASSERT(this->size <= size && "Data size is too large");

        VkDevice device = VulkanRenderer::getDevice();

        void* data;
        TK_ASSERT(vkMapMemory(device, memory, 0, size, 0, &data) == VK_SUCCESS);
        memcpy(data, dataIn, size);
        vkUnmapMemory(device, memory);
    }

    void  VulkanBuffer::cleanup() {
        VkDevice device = VulkanRenderer::getDevice();
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::create(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        VkDevice device = VulkanRenderer::getDevice();
        VkPhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        std::unique_ptr<VulkanBuffer> buffer(new VulkanBuffer(size));

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        TK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer->buffer) == VK_SUCCESS);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer->buffer, &memoryRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = VulkanDevice::findMemoryType(memoryRequirements.memoryTypeBits, properties);

        TK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, &buffer->memory) == VK_SUCCESS);
        vkBindBufferMemory(device, buffer->buffer, buffer->memory, 0);

        return buffer;
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::createVertexBuffer(uint32_t size, void* data) {
        std::unique_ptr<VulkanBuffer> buffer = create(
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        buffer->setData(size, data);
        return std::move(buffer);
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::createIndexBuffer(uint32_t size, void* data) {
        std::unique_ptr<VulkanBuffer> buffer = create(
            size,
             VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        buffer->setData(size, data);
        return std::move(buffer);
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::createUniformBuffer(uint32_t size, void* data) {
        std::unique_ptr<VulkanBuffer> buffer = create(
            size,
             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        buffer->setData(size, data);
        return std::move(buffer);
    }

}
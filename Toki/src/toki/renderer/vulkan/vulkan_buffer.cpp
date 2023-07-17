#include "vulkan_buffer.h"

#include "toki/core/application.h"

namespace Toki {

    VulkanBuffer::VulkanBuffer(uint32_t size) : size{ size } {}

    VulkanBuffer::~VulkanBuffer() {
        // cleanup();
    }

    void VulkanBuffer::setData(uint32_t size, void* dataIn) {
        TK_ASSERT(this->size >= size && "Data size is too large", "test");

        VkDevice device = Application::getVulkanRenderer()->getDevice();

        void* data;
        TK_ASSERT(vkMapMemory(device, memory, 0, size, 0, &data) == VK_SUCCESS);
        memcpy(data, dataIn, size);
        vkUnmapMemory(device, memory);
    }

    void VulkanBuffer::cleanup() {
        VkDevice device = Application::getVulkanRenderer()->getDevice();
        if (isMemoryMapped) unmapMemory();
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

    void* VulkanBuffer::mapMemory(uint32_t size) {
        void* data;
        TK_ASSERT(vkMapMemory(Application::getVulkanRenderer()->getDevice(), memory, 0, size, 0, &data) == VK_SUCCESS);
        isMemoryMapped = true;
        return data;
    }

    void VulkanBuffer::unmapMemory() {
        vkUnmapMemory(Application::getVulkanRenderer()->getDevice(), memory);
        isMemoryMapped = false;
    }

    std::shared_ptr<VulkanBuffer> VulkanBuffer::create(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        VkDevice device = Application::getVulkanRenderer()->getDevice();
        VkPhysicalDevice physicalDevice = Application::getVulkanRenderer()->getPhysicalDevice();

        std::shared_ptr<VulkanBuffer> buffer(new VulkanBuffer(size));

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

    std::shared_ptr<VulkanBuffer> VulkanBuffer::createVertexBuffer(uint32_t size, void* data) {
        std::shared_ptr<VulkanBuffer> buffer = create(
            size,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        if (data) buffer->setData(size, data);
        return buffer;
    }

    std::shared_ptr<VulkanBuffer> VulkanBuffer::createIndexBuffer(uint32_t size, void* data) {
        std::shared_ptr<VulkanBuffer> buffer = create(
            size,
             VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        if (data) buffer->setData(size, data);
        return buffer;
    }

    std::shared_ptr<VulkanBuffer> VulkanBuffer::createUniformBuffer(uint32_t size, void* data) {
        std::shared_ptr<VulkanBuffer> buffer = create(
            size,
             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        if (data) buffer->setData(size, data);
        return buffer;
    }

}
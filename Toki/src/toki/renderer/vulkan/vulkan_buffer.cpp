#include "vulkan_buffer.h"

namespace Toki {

    VulkanBuffer::VulkanBuffer(uint32_t size) : size{ size } {}

    void VulkanBuffer::setData(uint32_t size, void* dataIn) {
        TK_ASSERT(this->size <= size && "Data size is too large");

        vk::Device device = VulkanRenderer::getDevice();

        void* data;
        TK_ASSERT(device.mapMemory(memory, 0, size, {}, &data) == vk::Result::eSuccess);
        memcpy(data, dataIn, size);
        device.unmapMemory(memory);
    }

    void  VulkanBuffer::cleanup() {
        vk::Device device = VulkanRenderer::getDevice();
        device.destroyBuffer(buffer);
        device.freeMemory(memory);
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::create(uint32_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
        vk::Device device = VulkanRenderer::getDevice();
        vk::PhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        std::unique_ptr<VulkanBuffer> buffer(new VulkanBuffer(size));

        vk::BufferCreateInfo bufferInfo;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        TK_ASSERT(device.createBuffer(&bufferInfo, nullptr, &buffer->buffer) == vk::Result::eSuccess);

        vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer->buffer);

        vk::MemoryAllocateInfo allocInfo{};
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = VulkanDevice::findMemoryType(memoryRequirements.memoryTypeBits, properties);

        TK_ASSERT(device.allocateMemory(&allocInfo, nullptr, &buffer->memory) == vk::Result::eSuccess);
        device.bindBufferMemory(buffer->buffer, buffer->memory, 0);

        return buffer;
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::createVertexBuffer(uint32_t size, void* data) {
        std::unique_ptr<VulkanBuffer> buffer = create(
            size,
            vk::BufferUsageFlagBits::eVertexBuffer,
            static_cast<vk::MemoryPropertyFlags>(static_cast<uint32_t>(vk::MemoryPropertyFlagBits::eDeviceLocal) | static_cast<uint32_t>(vk::MemoryPropertyFlagBits::eHostVisible))
        );
        buffer->setData(size, data);
        return std::move(buffer);
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::createIndexBuffer(uint32_t size, void* data) {
        std::unique_ptr<VulkanBuffer> buffer = create(
            size,
            vk::BufferUsageFlagBits::eIndexBuffer,
            static_cast<vk::MemoryPropertyFlags>(static_cast<uint32_t>(vk::MemoryPropertyFlagBits::eDeviceLocal) | static_cast<uint32_t>(vk::MemoryPropertyFlagBits::eHostVisible))
        );
        buffer->setData(size, data);
        return std::move(buffer);
    }

    std::unique_ptr<VulkanBuffer> VulkanBuffer::createUniformBuffer(uint32_t size, void* data) {
        std::unique_ptr<VulkanBuffer> buffer = create(
            size,
            vk::BufferUsageFlagBits::eUniformBuffer,
            static_cast<vk::MemoryPropertyFlags>(static_cast<uint32_t>(vk::MemoryPropertyFlagBits::eDeviceLocal) | static_cast<uint32_t>(vk::MemoryPropertyFlagBits::eHostVisible))
        );
        buffer->setData(size, data);
        return std::move(buffer);
    }

}
#include "tkpch.h"
#include "vulkan_buffer.h"
#include "vulkan_renderer.h"
#include "core/assert.h"
#include "platform/vulkan/backend/vulkan_utils.h"

namespace Toki {

    VulkanBuffer::VulkanBuffer(const BufferConfig& config, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags properties) : config(config) {
        VkDevice device = VulkanRenderer::device();
        VkPhysicalDevice physicalDevice = VulkanRenderer::physicalDevice();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = config.size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        TK_ASSERT_VK_RESULT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer), "Could not create buffer");

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocInfo{};
        memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.allocationSize = memoryRequirements.size;
        memoryAllocInfo.memoryTypeIndex = VulkanUtils::findMemoryType(memoryRequirements.memoryTypeBits, properties);

        TK_ASSERT_VK_RESULT(vkAllocateMemory(device, &memoryAllocInfo, nullptr, &memory), "Could not allocate memory");
        TK_ASSERT_VK_RESULT(vkBindBufferMemory(device, buffer, memory, 0), "Could not bind buffer memory");
    }

    VulkanBuffer::~VulkanBuffer() {
        VkDevice device = VulkanRenderer::device();
        vkDeviceWaitIdle(device);
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

    void VulkanBuffer::setData(uint32_t dataSize, void* data) {
        TK_ASSERT(config.size >= dataSize, std::format("Buffer size is not large enough, size: {}, data size: {}", config.size, dataSize));

        VkDevice device = VulkanRenderer::device();
        TK_ASSERT_VK_RESULT(vkMapMemory(device, memory, 0, config.size, 0, &mappedData), "Could not map memory");
        memcpy(mappedData, data, dataSize);
        vkUnmapMemory(device, memory);
    }

#pragma region VulkanVertexBuffer

    VulkanVertexBuffer::VulkanVertexBuffer(const VertexBufferConfig& config)
        : VertexBuffer(config), VulkanBuffer(config, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) {
    }

    void VulkanVertexBuffer::bind() {
        VkDeviceSize offsets[] = { 0 };
        VkBuffer buffers[] = { buffer };
        vkCmdBindVertexBuffers(VulkanRenderer::commandBuffer(), binding, 1, buffers, offsets);
    }

    void VulkanVertexBuffer::setData(uint32_t dataSize, void* data) {
        VulkanBuffer::setData(dataSize, data);
    }

#pragma endregion VulkanVertexBuffer

#pragma region VulkanIndexBuffer

    VulkanIndexBuffer::VulkanIndexBuffer(const IndexBufferConfig& config)
        : IndexBuffer(config), VulkanBuffer(config, VK_BUFFER_USAGE_INDEX_BUFFER_BIT) {
    }

    void VulkanIndexBuffer::bind() {
        vkCmdBindIndexBuffer(VulkanRenderer::commandBuffer(), buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
    }

    void VulkanIndexBuffer::setData(uint32_t dataSize, void* data) {
        VulkanBuffer::setData(dataSize, data);
    }

#pragma endregion VulkanIndexBuffer

#pragma region VulkanUniformBuffer

    VulkanUniformBuffer::VulkanUniformBuffer(const UniformBufferConfig& config)
        : UniformBuffer(config), VulkanBuffer(config, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
    }

    void VulkanUniformBuffer::setData(uint32_t dataSize, void* data) {
        VulkanBuffer::setData(dataSize, data);
    }

#pragma endregion VulkanUniformBuffer

#pragma region VulkanStorageBuffer

    VulkanStorageBuffer::VulkanStorageBuffer(const BufferConfig& config)
        : StorageBuffer(config), VulkanBuffer(config, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
    }

    void VulkanStorageBuffer::setData(uint32_t dataSize, void* data) {
        VulkanBuffer::setData(dataSize, data);
    }

#pragma endregion VulkanStorageBuffer
}
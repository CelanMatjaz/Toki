#include "tkpch.h"
#include "vulkan_buffer.h"

#include "toki/core/application.h"

namespace Toki {

    VulkanBuffer::VulkanBuffer(VkDeviceSize size) : size(size) {}

    VulkanBuffer::~VulkanBuffer() {
        cleanup();
    }

    TkRef<VulkanBuffer> VulkanBuffer::create(VulkanBufferSpecification* spec) {
        VkDevice device = Application::getVulkanRenderer()->getDevice();
        VkPhysicalDevice physicalDevice = Application::getVulkanRenderer()->getPhysicalDevice();

        TkRef<VulkanBuffer> buffer = createRef<VulkanBuffer>(spec->size);

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = spec->size;
        bufferInfo.usage = spec->usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        TK_ASSERT(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer->buffer) == VK_SUCCESS);

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer->buffer, &memoryRequirements);

        VkMemoryAllocateInfo memoryAllocInfo{};
        memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.allocationSize = memoryRequirements.size;
        memoryAllocInfo.memoryTypeIndex = VulkanDevice::findMemoryType(memoryRequirements.memoryTypeBits, spec->properties);

        TK_ASSERT(vkAllocateMemory(device, &memoryAllocInfo, nullptr, &buffer->memory) == VK_SUCCESS);
        TK_ASSERT(vkBindBufferMemory(device, buffer->buffer, buffer->memory, 0) == VK_SUCCESS);

        return buffer;
    }

    TkRef<VulkanBuffer> VulkanBuffer::createStatic(VulkanBufferSpecification* spec, VulkanBufferData* data) {
        TK_ASSERT(spec->size == data->size);

        VulkanBufferSpecification stagingBufferSpec;
        stagingBufferSpec.size = data->size;
        stagingBufferSpec.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        stagingBufferSpec.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        auto stagingBuffer = create(&stagingBufferSpec);

        VulkanBufferSpecification finalBufferSpec;
        finalBufferSpec.size = spec->size;
        finalBufferSpec.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | spec->usage;
        finalBufferSpec.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        auto finalBuffer = create(&finalBufferSpec);

        copyBuffer(stagingBuffer, finalBuffer, spec->size);

        return finalBuffer;
    }

    void VulkanBuffer::copyBuffer(TkRef<VulkanBuffer> src, TkRef<VulkanBuffer> dst, uint32_t size) {
        VkCommandBuffer commandBuffer = Application::getVulkanRenderer()->startCommandBuffer();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, src->getBuffer(), dst->getBuffer(), 1, &copyRegion);

        Application::getVulkanRenderer()->endCommandBuffer(commandBuffer);
    }

    void VulkanBuffer::setData(VulkanBufferData* dataIn) {
        TK_ASSERT(this->size >= dataIn->size && "Buffer size is not large enough for the data provided in spec");

        VkDevice device = Application::getVulkanRenderer()->getDevice();

        void* data;
        TK_ASSERT(vkMapMemory(device, memory, 0, dataIn->size, 0, &data) == VK_SUCCESS);
        memcpy(data, dataIn->data, dataIn->size);
        vkUnmapMemory(device, memory);
    }

    void* VulkanBuffer::mapMemory() {
        if (isMemoryMapped) return mappedData;
        TK_ASSERT(vkMapMemory(Application::getVulkanRenderer()->getDevice(), memory, 0, size, 0, &mappedData) == VK_SUCCESS);
        isMemoryMapped = true;
        return mappedData;
    }

    void VulkanBuffer::unmapMemory() {
        TK_ASSERT(isMemoryMapped && "Memory needs to be mapped before in can be unmapped");
        vkUnmapMemory(Application::getVulkanRenderer()->getDevice(), memory);
        isMemoryMapped = false;
    }

    void VulkanBuffer::cleanup() {
        VkDevice device = Application::getVulkanRenderer()->getDevice();
        if (isMemoryMapped) unmapMemory();
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

}
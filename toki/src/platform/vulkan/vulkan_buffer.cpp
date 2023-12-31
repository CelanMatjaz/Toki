#include "vulkan_buffer.h"

#include <cstring>

#include "core/assert.h"
#include "core/core.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_utils.h"

namespace Toki {

Ref<VulkanBuffer> VulkanBuffer::create(const VulkanContext* context, const VulkanBufferConfig& config) {
    return createRef<VulkanBuffer>(context, config);
}

Ref<VulkanBuffer> VulkanBuffer::create(const VulkanContext* context, uint32_t size, void* data, bool isStatic) {
    return createRef<VulkanBuffer>(context, size, data, isStatic);
}

VulkanBuffer::VulkanBuffer(const VulkanContext* context, const VulkanBufferConfig& config) : context(context), config(config) {
    create();
}

VulkanBuffer::~VulkanBuffer() {
    destroy();
}

void VulkanBuffer::setData(uint32_t size, void* data) {
    TK_ASSERT_VK_RESULT(vkMapMemory(context->device, memory, 0, size, 0, &mappedMemory), "Could not map memory");
    memcpy(mappedMemory, data, size);
    vkUnmapMemory(context->device, memory);
    mappedMemory = nullptr;
}

void* VulkanBuffer::mapMemory() {
    if (isMemoryMapped) return mappedMemory;
    TK_ASSERT_VK_RESULT(vkMapMemory(context->device, memory, 0, config.size, 0, &mappedMemory), "Could not map memory");
    isMemoryMapped = true;
    return mappedMemory;
}

void VulkanBuffer::unmapMemory() {
    TK_ASSERT(isMemoryMapped, "Cannot unmap memory that is not mapped");
    vkUnmapMemory(context->device, memory);
}

void VulkanBuffer::create() {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = config.size;
    bufferCreateInfo.usage = config.usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    TK_ASSERT_VK_RESULT(vkCreateBuffer(context->device, &bufferCreateInfo, context->allocationCallbacks, &buffer), "Could not create buffer");

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(context->device, buffer, &memReqs);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memReqs.size;
    memoryAllocateInfo.memoryTypeIndex = findMemoryType(context->physicalDevice, memReqs.memoryTypeBits, config.properties);

    TK_ASSERT_VK_RESULT(vkAllocateMemory(context->device, &memoryAllocateInfo, context->allocationCallbacks, &memory), "Could not allocate memory");
    TK_ASSERT_VK_RESULT(vkBindBufferMemory(context->device, buffer, memory, 0), "Could not bind buffer memory");
}

void VulkanBuffer::destroy() {
    vkDeviceWaitIdle(context->device);
    vkDestroyBuffer(context->device, buffer, context->allocationCallbacks);
    vkFreeMemory(context->device, memory, context->allocationCallbacks);
}

}  // namespace Toki

#include "vulkan_buffer.h"

#include <cstring>

#include "renderer/vulkan/vulkan_utils.h"
#include "vulkan/vulkan_core.h"

namespace toki {

void VulkanBuffer::create(RendererContext* ctx, const Config& config) {
    m_size = config.size;

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = config.size;
    buffer_create_info.usage = config.usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(ctx->device, &buffer_create_info, ctx->allocation_callbacks, &m_handle), "Could not create buffer");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(ctx->device, m_handle, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = find_memory_type(ctx->physical_device, memory_requirements.memoryTypeBits, config.memory_properties);

    VK_CHECK(vkAllocateMemory(ctx->device, &memory_allocate_info, ctx->allocation_callbacks, &m_memory), "Could not allocate image memory");
    VK_CHECK(vkBindBufferMemory(ctx->device, m_handle, m_memory, 0), "Could not bind image memory");
}

void VulkanBuffer::destroy(RendererContext* ctx) {
    vkFreeMemory(ctx->device, m_memory, ctx->allocation_callbacks);
    m_memory = VK_NULL_HANDLE;
    vkDestroyBuffer(ctx->device, m_handle, ctx->allocation_callbacks);
    m_handle = VK_NULL_HANDLE;
}

void VulkanBuffer::set_data(RendererContext* ctx, u32 size, void* data) {
    void* mapped_data;
    vkMapMemory(ctx->device, m_memory, 0, size, 0, &mapped_data);
    std::memcpy(mapped_data, data, size);
    vkUnmapMemory(ctx->device, m_memory);
}

VkBuffer VulkanBuffer::get_handle() const {
    return m_handle;
}

u32 VulkanBuffer::get_size() const {
    return m_size;
}

}  // namespace toki

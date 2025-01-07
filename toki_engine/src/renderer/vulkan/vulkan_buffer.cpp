#include "vulkan_buffer.h"

#include "renderer/vulkan/vulkan_utils.h"
#include "vulkan/vulkan_core.h"

namespace toki {

vulkan_buffer vulkan_buffer_create(ref<renderer_context> ctx, const create_buffer_config& config) {
    vulkan_buffer buffer{};

    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = config.size;
    buffer_create_info.usage = config.usage;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(ctx->device, &buffer_create_info, ctx->allocation_callbacks, &buffer.handle), "Could not create buffer");

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(ctx->device, buffer.handle, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = find_memory_type(ctx->physical_device, memory_requirements.memoryTypeBits, config.memory_properties);

    VK_CHECK(vkAllocateMemory(ctx->device, &memory_allocate_info, ctx->allocation_callbacks, &buffer.memory), "Could not allocate image memory");
    VK_CHECK(vkBindBufferMemory(ctx->device, buffer.handle, buffer.memory, 0), "Could not bind image memory");

    return buffer;
}

void vulkan_buffer_destroy(ref<renderer_context> ctx, vulkan_buffer& buffer) {
    vkFreeMemory(ctx->device, buffer.memory, ctx->allocation_callbacks);
    buffer.memory = VK_NULL_HANDLE;
    vkDestroyBuffer(ctx->device, buffer.handle, ctx->allocation_callbacks);
    buffer.handle = VK_NULL_HANDLE;
}

}  // namespace toki

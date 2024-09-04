#include "buffer.h"

#include "vulkan/vulkan_core.h"

namespace Toki {

extern uint32_t find_memory_type(
    const VkPhysicalDeviceMemoryProperties& physical_device_memory_properties, uint32_t typeBits, VkMemoryPropertyFlags properties);

TkError create_buffer(VulkanState* state, const BufferConfig* buffer_config, RendererBuffer* renderer_buffer_out) {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_create_info.size = buffer_config->size;
    buffer_create_info.usage = buffer_config->usage;

    VkResult result = vkCreateBuffer(state->device, &buffer_create_info, state->allocation_callbacks, &renderer_buffer_out->buffer);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_RESOURCE_ERROR);

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(state->device, renderer_buffer_out->buffer, &memory_requirements);

    VkMemoryAllocateInfo memory_allocate_info{};
    memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex =
        find_memory_type(state->physical_device_data.memory_properties, memory_requirements.memoryTypeBits, renderer_buffer_out->memory_properties);

    result = vkAllocateMemory(state->device, &memory_allocate_info, state->allocation_callbacks, &renderer_buffer_out->memory);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_RESOURCE_ERROR);

    result = vkBindBufferMemory(state->device, renderer_buffer_out->buffer, renderer_buffer_out->memory, 0);
    ASSERT_VK_RESULT(result, Error::RENDERER_CREATE_RESOURCE_ERROR);

    return TkError{};
}

void destroy_buffer(VulkanState* state, RendererBuffer* renderer_buffer_out) {
    vkFreeMemory(state->device, renderer_buffer_out->memory, state->allocation_callbacks);
    vkDestroyBuffer(state->device, renderer_buffer_out->buffer, state->allocation_callbacks);
    renderer_buffer_out->buffer = VK_NULL_HANDLE;
}

}  // namespace Toki

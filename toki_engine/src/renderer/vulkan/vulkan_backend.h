#pragma once

#include <vulkan/vulkan.h>

#include <utility>

#include "containers/handle_map.h"
#include "core/base.h"
#include "engine/window.h"
#include "memory/allocators/basic_allocator.h"
#include "memory/allocators/double_buffer_allocator.h"
#include "memory/allocators/stack_allocator.h"
#include "renderer/renderer_commands.h"
#include "renderer/renderer_structs.h"
#include "renderer/renderer_types.h"
#include "renderer/vulkan/vulkan_commands.h"
#include "renderer/vulkan/vulkan_types.h"
#include "resources/configs/shader_config_loader.h"

namespace toki {

namespace vulkan_renderer {

struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties physical_device_properties;
    VkDevice device;
    Queue graphics_queue, present_queue;

    Swapchain swapchains[MAX_SWAPCHAIN_COUNT];
    containers::HandleMap<RenderPass> render_passes;
    containers::HandleMap<Pipeline> pipelines;
    containers::HandleMap<VulkanBuffer> buffers;

    VkAllocationCallbacks* allocation_callbacks;

    b8 vsync_enabled;
    u8 submit_count;

    BasicRef<VkCommandPool> command_pools;
    BasicRef<VkCommandPool> extra_command_pools;

    CommandBuffers command_buffers[MAX_FRAMES_IN_FLIGHT];

    InternalBuffer staging_buffer{};

    containers::HandleMap<VulkanImage> images;

    VkClearColorValue color_clear{};
    VkClearDepthStencilValue depth_stencil_clear{};
};

class VulkanBackend {
public:
    VulkanBackend();
    ~VulkanBackend();

    void create_device(Window* window);

    Handle create_swapchain(Window* window);
    void destroy_swapchain(Handle swapchain_handle);
    void recreate_swapchain(Swapchain* swapchain);

    Buffer create_buffer(BufferType type, u32 size);
    void destroy_buffer(Buffer* buffer);
    void* map_buffer_memory(VkDeviceMemory memory, u32 offset, u32 size);
    void unmap_buffer_memory(VkDeviceMemory memory);
    void flush_buffer(Buffer* buffer);
    void set_bufffer_data(Buffer* buffer, u32 size, void* data);
    void copy_buffer_data(VkBuffer dst, VkBuffer src, u32 size, u32 dst_offset = 0, u32 src_offset = 0);

    Handle create_image(ColorFormat format, u32 width, u32 height);
    void destroy_image(Handle image_handle);

    Handle create_render_pass();
    void destroy_render_pass(Handle render_pass_handle);
    void recreate_framebuffers(RenderPass* render_pass, Swapchain* swapchain, b8 destroy = true);

    Shader create_pipeline(Handle render_pass_handle, const configs::ShaderConfig& config);
    void destroy_pipeline(Handle pipeline_handle);

    void initialize_resources();
    void cleanup_resources();

    void wait_for_resources();

    void prepare_frame_resources();
    void cleanup_frame_resources();
    void submit_commands();
    void present();

    VkCommandBuffer get_command_buffer();
    RendererCommands* get_commands();

    void set_color_clear(const glm::vec4& color);
    void set_depth_clear(f32 depth_clear);
    void set_stencil_clear(u32 stencil_clear);

    // Draw commands
    void begin_render_pass(VkCommandBuffer cmd, const Rect2D& render_area);
    void end_render_pass(VkCommandBuffer cmd);

    void bind_shader(VkCommandBuffer cmd, Shader const& shader);
    void bind_buffer(VkCommandBuffer cmd, Buffer const& buffer);
    void draw(VkCommandBuffer cmd, u32 count);
    void draw_indexed(VkCommandBuffer cmd, u32 count);
    void draw_instanced(VkCommandBuffer cmd, u32 index_count, u32 instance_count = 1);

private:
    void create_instance();
    void find_physical_device(VkSurfaceKHR surface);

    VulkanBuffer create_buffer_internal(u32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
    void destroy_buffer_internal(VulkanBuffer* buffer);
    VulkanImage create_image_internal(
        VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags memory_properties);
    void destroy_image_internal(VulkanImage* image);

    void prepare_swapchain_frame(Swapchain* swapchain);
    void reset_swapchain_frame(Swapchain* swapchain);

    b8 submit_frame_command_buffers();
    VkCommandBuffer start_single_use_command_buffer();
    void submit_single_use_command_buffer(VkCommandBuffer cmd);

    FrameData* get_current_frame();
    CommandBuffers* get_current_command_buffers();

    u32 find_memory_type_index(u32 type_filter, VkMemoryPropertyFlags properties);
    PipelineResources create_pipeline_resources(const std::vector<configs::Shader>& stages);
    static std::vector<u32> create_shader_binary(configs::Shader shader);

    static void reflect_shader(
        ShaderStage stage,
        std::vector<u32>& binary,
        DescriptorBindings& bindings,
        std::vector<VkPushConstantRange>& push_constants);

private:
    BasicAllocator m_allocator{ 1024, Megabytes(64) };        // Allocate 64 megabytes for long lived allocations
    StackAllocator m_tempAllocator{ Megabytes(10) };          // Allocate 10 megabytess for temporary allocations
    DoubleBufferAllocator m_frameAllocator{ Megabytes(20) };  // Allocate 2 * 20 megabytes for frame allocations
    VulkanContext m_context{};
    Frames m_frames{};
    u32 m_inFlightFrameIndex{};
};

}  // namespace vulkan_renderer

}  // namespace toki

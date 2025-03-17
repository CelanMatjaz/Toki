#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include "../renderer_commands.h"
#include "vulkan_commands.h"
#include "vulkan_internal_types.h"
#include "vulkan_types.h"

namespace toki {

struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkPhysicalDeviceProperties physical_device_properties;
    VkDevice device;
    Queue graphics_queue, present_queue;
    Limits limits;
    DeviceProperties properties;

    VkAllocationCallbacks* allocation_callbacks;
};

struct VulkanResources {
    VulkanResources(AllocatorConcept auto& allocator):
        swapchains(allocator, 1),
        buffers(allocator, MAX_LOADED_BUFFER_COUNT),
        images(allocator, MAX_LOADED_BUFFER_COUNT),
        shaders(allocator, MAX_LOADED_BUFFER_COUNT),
        framebuffers(allocator, MAX_LOADED_BUFFER_COUNT) {}

    InternalBuffer staging_buffer{};
    u64 staging_buffer_offset{};

    HandleMap<Swapchain> swapchains;
    HandleMap<InternalBuffer> buffers;
    HandleMap<InternalImage> images;
    HandleMap<InternalShader> shaders;
    HandleMap<InternalFramebuffer> framebuffers;

    BasicRef<VkCommandPool> command_pools;
    BasicRef<VkCommandPool> extra_command_pools;
    CommandBuffers command_buffers[MAX_FRAMES_IN_FLIGHT];
};

struct VulkanSettings {
    u8 swapchain_count : 4 {};
    b8 vsync_enabled : 1 { false };

    VkClearColorValue color_clear{};
    VkClearDepthStencilValue depth_stencil_clear{ 1.0f, 0 };
};

class VulkanBackend {
public:
    VulkanBackend();
    ~VulkanBackend();

    void create_device();

    void create_swapchain(NativeWindowHandle handle);
    void destroy_swapchain(Handle& window_data_handle);
    void recreate_swapchain(Swapchain& swapchain);

    Handle create_framebuffer(
        u32 width, u32 height, u32 color_attachment_count, ColorFormat format, b8 has_depth, b8 has_stencil);
    void destroy_framebuffer(Handle framebuffer_handle);

    Handle create_buffer(BufferType type, u32 size);
    void destroy_buffer(Buffer& buffer);
    void* map_buffer_memory(VkDeviceMemory memory, u32 offset, u32 size);
    void unmap_buffer_memory(VkDeviceMemory memory);
    void flush_buffer(Buffer& buffer);
    void set_buffer_data(const Buffer& buffer, u32 size, void* data);
    void copy_buffer_data(VkBuffer dst, VkBuffer src, u32 size, u32 dst_offset = 0, u32 src_offset = 0);

    Handle create_image(ColorFormat format, u32 width, u32 height);
    void destroy_image(Handle image_handle);

    Handle create_shader_internal(const Framebuffer& framebuffer, const ShaderConfig& config);
    void destroy_shader_internal(Handle shader_handle);

    void initialize_resources();
    void cleanup_resources();

    void wait_for_resources();

    void prepare_frame_resources();
    void cleanup_frame_resources();
    void submit_commands();
    void present();

    VkCommandBuffer get_command_buffer();
    RendererCommands* get_commands();

    void set_color_clear(const Vec4<f32>& color);
    void set_depth_clear(f32 depth_clear);
    void set_stencil_clear(u32 stencil_clear);

    InternalShader* get_shader(Handle handle);

    // Draw commands
    void begin_rendering(VkCommandBuffer cmd, Handle framebuffer_handle, const Rect2D& render_area);
    void end_rendering(VkCommandBuffer cmd, Handle framebuffer_handle);

    void bind_shader(VkCommandBuffer cmd, Shader const& shader);
    void bind_buffer(VkCommandBuffer cmd, Buffer const& buffer);
    void draw(VkCommandBuffer cmd, u32 count);
    void draw_indexed(VkCommandBuffer cmd, u32 count);
    void draw_instanced(VkCommandBuffer cmd, u32 index_count, u32 instance_count = 1);

    void push_constants(
        VkCommandBuffer cmd,
        VkPipelineLayout layout,
        VkShaderStageFlags stage_flags,
        u32 offset,
        u32 size,
        const void* data);

private:
    void create_instance();
    void find_physical_device(VkSurfaceKHR surface);

    InternalBuffer create_buffer_internal(u32 size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties);
    void destroy_buffer_internal(InternalBuffer* buffer);

    InternalImage create_image_internal(
        u32 width,
        u32 height,
        u32 layer_count,
        VkFormat format,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags memory_properties,
        VkImageAspectFlags aspect_flags);
    void destroy_image_internal(InternalImage* image);

    void prepare_swapchain_frame(Swapchain* swapchain);
    void reset_swapchain_frame(Swapchain* swapchain);

    b8 submit_frame_command_buffers();
    VkCommandBuffer start_single_use_command_buffer();
    void submit_single_use_command_buffer(VkCommandBuffer cmd);

    FrameData* get_current_frame();
    CommandBuffers* get_current_command_buffers();

    void transition_framebuffer_images(VkCommandBuffer cmd, InternalFramebuffer* framebuffer);
    void transition_swapchain_image(VkCommandBuffer cmd, Swapchain* swapchain);

    void transition_image_layout(const TransitionLayoutConfig& config, InternalImage* image);
    void transition_image_layout(
        VkCommandBuffer cmd, const TransitionLayoutConfig& config, VkImageAspectFlags aspect_flags, VkImage image);
    void transition_image_layout(VkCommandBuffer cmd, const TransitionLayoutConfig& config, InternalImage* image);
    void transition_image_layouts(
        VkCommandBuffer cmd,
        const TransitionLayoutConfig& config,
        VkImageAspectFlags aspect_flags,
        VkImage* images,
        u32 image_count);

    Pipeline create_pipeline_internal(
        std::string_view config_path,
        VkFormat format,
        u64 attachment_count,
        VkFormat depth_format,
        VkFormat stencil_format);
    void destroy_pipeline_internal(Pipeline* pipeline);
    // PipelineResources create_pipeline_resources(const std::vector<configs::Shader>& stages);
    // ShaderModule create_shader_module(ShaderStage stage, std::string_view path);

    u32 find_memory_type_index(u32 type_filter, VkMemoryPropertyFlags properties);
    VkImageMemoryBarrier create_image_memory_barrier(
        VkImageLayout old_layout, VkImageLayout new_layout, VkImageAspectFlags aspect_flags);

    const Limits& limits() const;
    const DeviceProperties& device_properties() const;
    VkImage get_swapchain_image(u32 swapchain_index);
    VkImageView get_swapchain_image_view(u32 swapchain_index);

private:
    Allocator mAllocator;
    BasicRef<VulkanContext> mContext;
    BasicRef<VulkanResources> mResources;
    BasicRef<VulkanSettings> mSettings;
    FrameData mFrames[MAX_FRAMES_IN_FLIGHT];
    u32 mInFlightFrameIndex{};
    DoubleBumpAllocator mFrameAllocator;
};

}  // namespace toki

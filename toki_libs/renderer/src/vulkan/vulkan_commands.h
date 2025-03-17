#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include "../renderer_commands.h"

namespace toki {

class VulkanBackend;

class VulkanCommands : public RendererCommands {
public:
    VulkanCommands() = delete;
    VulkanCommands(VulkanBackend* backend);

    virtual void begin_rendering(const Framebuffer& framebuffer, const Rect2D& render_area) override;
    virtual void end_rendering() override;

    virtual void set_viewport(const Rect2D& rect) override;
    virtual void reset_viewport() override;
    virtual void set_scissor(const Rect2D& rect) override;
    virtual void reset_scissor() override;

    virtual void bind_shader(const Shader& shader) override;
    virtual void bind_buffer(const Buffer& buffer) override;
    virtual void bind_texture(const Texture& buffer) override;

    virtual void push_constants(u32 offset, u32 size, const void* data) override;

    virtual void draw(u32 count) override;
    virtual void draw_indexed(u32 count) override;
    virtual void draw_instanced(u32 index_count, u32 instance_count) override;

private:
    VulkanBackend* mBackend;
    VkCommandBuffer mCommandBuffer;
    Handle mFramebufferHandle;
    Handle mShaderHandle;
};

}  // namespace toki

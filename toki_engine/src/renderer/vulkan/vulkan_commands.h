#pragma once

#include <vulkan/vulkan.h>

#include "core/base.h"
#include "renderer/renderer_commands.h"
#include "renderer/vulkan/vulkan_types.h"

namespace toki {

namespace renderer {

class VulkanBackend;

class VulkanCommands : public RendererCommands {
public:
    VulkanCommands() = delete;
    VulkanCommands(VulkanBackend* backend);

    virtual void begin_rendering(const Framebuffer* framebuffer, const Rect2D& render_area) override;
    virtual void end_rendering() override;

    virtual void set_viewport(const Rect2D& rect) override;
    virtual void reset_viewport() override;
    virtual void set_scissor(const Rect2D& rect) override;
    virtual void reset_scissor() override;

    virtual void bind_shader(Shader const& shader) override;
    virtual void bind_buffer(Buffer const& buffer) override;

    virtual void push_constants(u32 offset, u32 size, const void* data) override;

    virtual void draw(u32 count) override;
    virtual void draw_indexed(u32 count) override;
    virtual void draw_instanced(u32 index_count, u32 instance_count) override;

private:
    VulkanBackend* m_backend;
    VkCommandBuffer m_commandBuffer;
    Handle m_framebufferHandle;
    Handle m_shaderHandle;
};

}  // namespace renderer

}  // namespace toki

#pragma once

#include "core/core.h"
#include "filesystem"
#include "renderer/renderer_types.h"
#include "vector"
#include "vulkan/vulkan_core.h"

namespace Toki {

class VulkanFramebuffer;

#define MAX_FRAMES 3

struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t graphicsFamilyIndex = UINT32_MAX;
    uint32_t presentFamilyIndex = UINT32_MAX;

    VkAllocationCallbacks* allocationCallbacks = nullptr;
};

struct SwapchainConfig {};

enum class AttachmentTarget {
    Swapchain,
    Texture
};

enum class AttachmentType {
    None,
    Color,
    Depth,
    Stencil,
    DepthStencil
};

enum class SampleCount {
    SampleCount1,
    SampleCount2,
    SampleCount4,
    SampleCount8,
    SampleCount16,
    SampleCount32,
    SampleCount64
};

enum class AttachmentLoadOp {
    Load,
    Clear,
    DontCare
};

enum class AttachmentStoreOp {
    Store,
    DontCare
};

struct RenderPassAttachment {
    AttachmentTarget target = AttachmentTarget::Swapchain;
    AttachmentType type = AttachmentType::None;
    SampleCount sampleCount = SampleCount::SampleCount1;
    Format format = Format::None;
    AttachmentLoadOp loadOp;
    AttachmentStoreOp storeOp;
};

struct RenderPassConfig {
    std::vector<RenderPassAttachment> attachments;
    uint32_t subpassCount = 0;
};

enum class PrimitiveTopology {
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    LineListWithAdjecency,
    LineStripWithAdjecency,
    TriangleListWithAdjecency,
    TriangleStripWithAdjecency,
    PatchList
};

enum class PolygonMode {
    Fill,
    Line,
    Point
};

enum class CullMode {
    None,
    Front,
    Back,
    FrontAndBack
};

enum class FrontFace {
    Clockwise,
    CounterClockwise
};

struct PipelineConfig {
    // PipelineType pipelineType = PipelineType::None;
    std::filesystem::path shaderPath;
};

struct FramebufferConfig {
    uint32_t width, height, layers = 1;
    std::vector<VkImageView> attachments;
    VkRenderPass renderPass = VK_NULL_HANDLE;
};

struct ImageViewConfig {
    VkImage image = VK_NULL_HANDLE;
    VkFormat format = VK_FORMAT_MAX_ENUM;
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_MAX_ENUM;
};

struct FrameData {
    VkCommandPool commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> secondaryCommandBuffers;
    uint32_t secondaryCommandBufferRecordingCount = 0;

    VkSemaphore presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderSemaphore = VK_NULL_HANDLE;
    VkFence renderFence = VK_NULL_HANDLE;

    std::vector<Ref<VulkanFramebuffer>> framebuffers;

    uint32_t currentImageIndex = 0;
    uint32_t nDrawCommands = 0;
    uint32_t nDrawnMeshes = 0;
};

}  // namespace Toki

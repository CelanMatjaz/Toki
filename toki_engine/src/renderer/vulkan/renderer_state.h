#pragma once

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <vector>

#include "core/core.h"
#include "renderer/renderer.h"
#include "renderer/vulkan/data/attachment_hash.h"
#include "renderer/vulkan/data/render_pass.h"

namespace toki {

inline const std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

struct Frame {
    VkCommandBuffer commandBuffer{};
};

struct RenderPassWithRefCount {
    RenderPass renderPass;
    u64 refCount{};
};

struct RendererContext {
    VkInstance instance{};
    VkDevice device{};
    VkPhysicalDevice physicalDevice{};

    VkAllocationCallbacks* allocationCallbacks{};

    u32 currentFrameIndex{};
    Frame frames[ERROR_COUNT]{};

    std::unordered_map<AttachmentsHash, RenderPassWithRefCount, AttachmentsHash> renderPassMap;
};

}  // namespace toki

#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

#include "toki/core/assert.h"
#include "toki/renderer/renderer_types.h"

#define MAX_FRAMES 3

namespace Toki {

class AttachmentFormatHash {
public:
    AttachmentFormatHash() : m_data(0) {}
    AttachmentFormatHash(const std::vector<Attachment>& attachments) : m_data(0), m_attachmentCount(attachments.size()) {
        uint32_t i = 0;
        for (const auto& attachment : attachments) {
            switch (attachment.colorFormat) {
                case ColorFormat::R8:
                case ColorFormat::RG8:
                case ColorFormat::RGBA8:
                    m_data.u8[i] = (uint32_t) attachments[i].colorFormat;
                    ++i;

                default:
            }
        }

        TK_ASSERT(i <= 8, "Max 8 color attachments supported");
    }

    bool operator==(const AttachmentFormatHash& other) const { return m_data.u64 == other.m_data.u64; }
    bool operator<(const AttachmentFormatHash& other) const { return m_data.u64 < other.m_data.u64; }
    uint64_t operator()(const AttachmentFormatHash& h) const { return h.m_data.u64; }

    operator bool() const { return m_data.u64 != 0 && m_attachmentCount > 0; }

private:
    union {
        uint64_t u64;
        uint8_t u8[8];
    } m_data;

    uint8_t m_attachmentCount = 0;
};

struct PhysicalDeviceData {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    std::vector<VkQueueFamilyProperties> queueFamilyProperties;
    std::vector<VkPresentModeKHR> presentModes;
    std::vector<VkBool32> supportsPresent;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkPhysicalDeviceMemoryProperties memoryProperties;

    VkSurfaceFormatKHR presentableSurfaceFormat;
    VkPresentModeKHR presentMode;

    std::optional<uint32_t> graphicsFamilyIndex;
    std::optional<uint32_t> presentFamilyIndex;
    std::optional<uint32_t> transferFamilyIndex;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkQueue transferQueue;
};

struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    PhysicalDeviceData physicalDeviceData{};

    VkAllocationCallbacks* allocationCallbacks = nullptr;

    operator VkInstance() { return instance; }
    operator VkDevice() { return device; }
    operator VkPhysicalDevice() { return physicalDeviceData.physicalDevice; }
};

inline VulkanContext context;

struct FrameContext {
    VkSemaphore presentSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderSemaphore = VK_NULL_HANDLE;
    VkFence renderFence = VK_NULL_HANDLE;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
};

}  // namespace Toki

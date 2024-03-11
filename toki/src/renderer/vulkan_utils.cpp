#include "vulkan_utils.h"

#include <utility>

#include "platform.h"
#include "toki/core/assert.h"
#include "vulkan/vulkan_core.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include <GLFW/glfw3.h>
#endif

namespace Toki {

VkSurfaceKHR VulkanUtils::createSurface(Ref<VulkanContext> context, Ref<Window> window) {
    VkSurfaceKHR surface = VK_NULL_HANDLE;

#ifdef TK_WINDOW_SYSTEM_GLFW
    TK_ASSERT_VK_RESULT(
        glfwCreateWindowSurface(context->instance, (GLFWwindow*) window->getHandle(), context->allocationCallbacks, &surface),
        "Could not create surface");
#endif

    TK_ASSERT(surface != VK_NULL_HANDLE, "Surface should not be VK_NULL_HANDLE");

    return surface;
}

bool VulkanUtils::checkForMailboxPresentModeSupport(Ref<VulkanContext> context, VkSurfaceKHR surface) {
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    if (presentModeCount > 0) {
        vkGetPhysicalDeviceSurfacePresentModesKHR(context->physicalDevice, surface, &presentModeCount, nullptr);
    }

    for (const auto& presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return true;
        }
    }

    return false;
}

VkImageAspectFlags VulkanUtils::getImageAspectFlags(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            std::unreachable();
    }
}

uint32_t VulkanUtils::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    TK_ASSERT(false, "Failed to find suitable memory type");

    return -1;
}

VkShaderStageFlagBits VulkanUtils::mapShaderStage(ShaderStage shaderStage) {
    switch (shaderStage) {
        case ShaderStage::SHADER_STAGE_VERTEX:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::SHADER_STAGE_FRAGMENT:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        default:
            std::unreachable();
    }
}

VkFormat VulkanUtils::mapFormat(ColorFormat colorFormat) {
    switch (colorFormat) {
        case ColorFormat::COLOR_FORMAT_RGBA:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::COLOR_FORMAT_DEPTH:
            return VK_FORMAT_D32_SFLOAT;
        case ColorFormat::COLOR_FORMAT_STENCIL:
            return VK_FORMAT_S8_UINT;
        case ColorFormat::COLOR_FORMAT_DEPTH_STENCIL:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        default:
            std::unreachable();
    }
}

}  // namespace Toki

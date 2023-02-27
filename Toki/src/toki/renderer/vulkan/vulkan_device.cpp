#include "vulkan_device.h"

#include "tkpch.h"
#include "toki/core/application.h"
#include "vulkan_constants.h"

namespace Toki {

    vk::Device VulkanDevice::createDevice() {
        float queuePriority = 0.0f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo{{}, VulkanDevice::queueFamilyIndexes.graphicsQueueIndex, 1, & queuePriority};

        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = true;
        deviceFeatures.wideLines = true;
        deviceFeatures.multiViewport = true;
        deviceFeatures.samplerAnisotropy = true;

        vk::DeviceCreateInfo deviceCreateInfo { {}, deviceQueueCreateInfo, {}, deviceExtensions, & deviceFeatures };

#ifndef DIST
        TK_ASSERT(VulkanDevice::checkForValidationLayerSupport());
        deviceCreateInfo.setPEnabledLayerNames(validationLayers);
#endif

        VkDevice device;
        vkCreateDevice(VulkanRenderer::getPhysicalDevice(), reinterpret_cast<VkDeviceCreateInfo*>(&deviceCreateInfo), nullptr, &device);
        return vk::Device { device };
    }

    bool VulkanDevice::checkForValidationLayerSupport() {
        std::vector<vk::LayerProperties> instanceLayerProperties = vk::enumerateInstanceLayerProperties();

        for (const auto& layerName : validationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : instanceLayerProperties) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) return false;
        }

        return true;
    }

    void VulkanDevice::initQueueFamilyIndexes() {
        vk::SurfaceKHR surface = VulkanRenderer::getSurface();
        vk::PhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

        bool foundIndecies = false;

        for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
            if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                VkBool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);

                if (!presentSupport) continue;

                queueFamilyIndexes.graphicsQueueIndex = i;
                queueFamilyIndexes.presentQueueIndex = i;

                foundIndecies = true;
            }
        }

        if (!foundIndecies) {
            bool graphicsFound = false;
            bool presentFound = false;

            for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
                if (!graphicsFound && queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
                    queueFamilyIndexes.graphicsQueueIndex = i;
                    bool graphicsFound = true;
                }

                VkBool32 presentSupport = physicalDevice.getSurfaceSupportKHR(i, surface);

                if (!presentFound && presentSupport) {
                    queueFamilyIndexes.presentQueueIndex = i;
                    bool presentFound = true;
                }

                if (graphicsFound && presentFound)
                    break;
            }
        }

        TK_ASSERT(queueFamilyIndexes.presentQueueIndex < UINT32_MAX && queueFamilyIndexes.graphicsQueueIndex < UINT32_MAX);
    }


    vk::Extent2D VulkanDevice::getExtent(vk::SurfaceCapabilitiesKHR capabilities) {
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            vk::Extent2D extent;
            extent.width = std::clamp(windowWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(windowHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return extent;
        }
        else {
            return capabilities.currentExtent;
        }
    }

    uint32_t VulkanDevice::getImageCount(vk::SurfaceCapabilitiesKHR capabilities, uint32_t minImageCount) {
        if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
            return capabilities.maxImageCount;
        }
        return capabilities.minImageCount;
    }

    vk::SurfaceTransformFlagBitsKHR VulkanDevice::getPreTransform(vk::SurfaceCapabilitiesKHR capabilities) {
        return (capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            ? vk::SurfaceTransformFlagBitsKHR::eIdentity
            : capabilities.currentTransform;
    }

    vk::CompositeAlphaFlagBitsKHR VulkanDevice::getCompositeAlpha(vk::SurfaceCapabilitiesKHR capabilities) {
        return (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
            : (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
            : (capabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit
            : vk::CompositeAlphaFlagBitsKHR::eOpaque;
    }

    vk::ImageTiling VulkanDevice::findTiling(vk::Format format) {
        vk::FormatProperties formatProperties = VulkanRenderer::getPhysicalDevice().getFormatProperties(format);

        if (formatProperties.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            return vk::ImageTiling::eLinear;
        }
        else if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
            return vk::ImageTiling::eOptimal;
        }
        else {
            TK_ASSERT(false && "Format not supported");
        }
    }

    vk::Format VulkanDevice::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlagBits features) {
        vk::PhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        for (const vk::Format& format : candidates) {
            vk::FormatProperties props = physicalDevice.getFormatProperties(format);
            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        TK_ASSERT(false && "Failed to find suitable format");
    }

    vk::SurfaceFormatKHR VulkanDevice::findSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
        vk::PhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        for (const auto& availableFormat : formats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return formats[0];
    }

    uint32_t VulkanDevice::findMemoryType(uint32_t typeBits) {
        vk::PhysicalDeviceMemoryProperties memoryProperties = VulkanRenderer::getPhysicalDevice().getMemoryProperties();

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeBits & 1) &&
                 ((memoryProperties.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal) == vk::MemoryPropertyFlagBits::eDeviceLocal)) {
                return i;
            }
            typeBits >>= 1;
        }

        TK_ASSERT(false && "Failed to find suitable memory type");
    }
}
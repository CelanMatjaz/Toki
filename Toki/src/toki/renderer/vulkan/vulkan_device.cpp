#include "vulkan_device.h"

#include "tkpch.h"
#include "toki/core/application.h"
#include "vulkan_constants.h"

namespace Toki {

    VkDevice VulkanDevice::createDevice() {
        float queuePriority = 0.0f;
        VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.queueFamilyIndex = VulkanDevice::queueFamilyIndexes.graphicsQueueIndex;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = true;
        deviceFeatures.wideLines = true;
        deviceFeatures.multiViewport = true;
        deviceFeatures.samplerAnisotropy = true;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

#ifndef DIST
        TK_ASSERT(VulkanDevice::checkForValidationLayerSupport());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        deviceCreateInfo.enabledLayerCount = validationLayers.size();
#endif

        VkDevice device;
        vkCreateDevice(VulkanRenderer::getPhysicalDevice(), &deviceCreateInfo, nullptr, &device);
        return device;
    }

    bool VulkanDevice::checkForValidationLayerSupport() {
        uint32_t layerPropCount;
        vkEnumerateInstanceLayerProperties(&layerPropCount, nullptr);
        std::vector<VkLayerProperties> instanceLayerProperties(layerPropCount);
        vkEnumerateInstanceLayerProperties(&layerPropCount, instanceLayerProperties.data());

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
        VkSurfaceKHR surface = VulkanRenderer::getSurface();
        VkPhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        uint32_t propCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(propCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &propCount, queueFamilyProperties.data());

        bool foundIndecies = false;

        for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {

                VkBool32 presentSupport;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

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
                if (!graphicsFound && queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    queueFamilyIndexes.graphicsQueueIndex = i;
                    bool graphicsFound = true;
                }

                VkBool32 presentSupport;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

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

    void VulkanDevice::initQueueFamilyProperties() {
        uint32_t count{};
        vkGetPhysicalDeviceQueueFamilyProperties(VulkanRenderer::getPhysicalDevice(), &count, nullptr);
        queueFamilyProperties.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(VulkanRenderer::getPhysicalDevice(), &count, queueFamilyProperties.data());
    }

    std::vector<VkPhysicalDevice> VulkanDevice::enumeratePhysicalDevices() {
        VkInstance instance = VulkanRenderer::getInstance();

        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

        return physicalDevices;
    }


    VkExtent2D VulkanDevice::getExtent(VkSurfaceCapabilitiesKHR capabilities) {
        const auto [windowWidth, windowHeight] = Application::getWindow()->getWindowDimensions();

        if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            VkExtent2D extent;
            extent.width = std::clamp(windowWidth, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(windowHeight, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return extent;
        }
        else {
            return capabilities.currentExtent;
        }
    }

    uint32_t VulkanDevice::getImageCount(VkSurfaceCapabilitiesKHR capabilities, uint32_t minImageCount) {
        if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
            return capabilities.maxImageCount;
        }
        return capabilities.minImageCount;
    }

    VkSurfaceTransformFlagBitsKHR VulkanDevice::getPreTransform(VkSurfaceCapabilitiesKHR capabilities) {
        return (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
            : capabilities.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR VulkanDevice::getCompositeAlpha(VkSurfaceCapabilitiesKHR capabilities) {
        return (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
            : (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) ? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
            : (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
            : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    VkImageTiling VulkanDevice::findTiling(VkFormat format) {
        VkPhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

        if (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return VK_IMAGE_TILING_LINEAR;
        }
        else if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return VK_IMAGE_TILING_OPTIMAL;
        }
        else {
            TK_ASSERT(false && "Format not supported");
        }
    }

    VkFormat VulkanDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlagBits features) {
        VkPhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        for (const VkFormat& format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        TK_ASSERT(false && "Failed to find suitable format");
    }

    VkSurfaceFormatKHR VulkanDevice::findSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
        VkPhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        for (const auto& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return formats[0];
    }

    uint32_t VulkanDevice::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) {
        VkPhysicalDevice physicalDevice = VulkanRenderer::getPhysicalDevice();

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((typeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        TK_ASSERT(false && "Failed to find suitable memory type");
    }
}
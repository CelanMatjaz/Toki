#include "vulkan_renderer.h"

#include <print>
#include <vector>

#include "platform.h"
#include "renderer/vulkan_utils.h"
#include "toki/core/assert.h"

#ifdef TK_WINDOW_SYSTEM_GLFW
#include "GLFW/glfw3.h"
#endif

namespace Toki {

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};
const std::vector<const char*> requiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

bool checkDeviceExtensionSupport(Ref<VulkanContext> context);
bool checkValidationLayerSupport();

VulkanRenderer::VulkanRenderer() : m_context(createRef<VulkanContext>()) {}

void VulkanRenderer::init() {
    createInstance();

    // Select physical device
    {
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(m_context->instance, &physicalDeviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(m_context->instance, &physicalDeviceCount, physicalDevices.data());
        m_context->physicalDevice = physicalDevices[0];
    }

    // Create device
    {
        WindowConfig windowConfig{};
        windowConfig.showOnCreate = false;
        Ref<Window> tempWindow = Window::create(windowConfig);
        VkSurfaceKHR tempSurface = VulkanUtils::createSurface(m_context, tempWindow);
        createDevice(tempSurface);
        vkDestroySurfaceKHR(m_context->instance, tempSurface, m_context->allocationCallbacks);
    }
}

void VulkanRenderer::shutdown() {
    m_swapchains.clear();

    vkDestroyDevice(m_context->device, m_context->allocationCallbacks);
    vkDestroyInstance(m_context->instance, m_context->allocationCallbacks);
}

void VulkanRenderer::createSwapchain(Ref<Window> window) {
    m_swapchains.emplace_back(VulkanSwapchain::create(m_context, {}, window));
}

void VulkanRenderer::createInstance() {
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "Toki";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "Toki";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

#ifdef TK_WINDOW_SYSTEM_GLFW
    uint32_t extensionCount = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
#endif

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.enabledExtensionCount = extensionCount;
    instanceCreateInfo.ppEnabledExtensionNames = extensions;

#ifndef DIST
    if (checkValidationLayerSupport()) {
        instanceCreateInfo.enabledLayerCount = validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        TK_ASSERT(false, "1 or more validation layers not supported by the system");
    }
#endif  // !DIST

    TK_ASSERT_VK_RESULT(vkCreateInstance(&instanceCreateInfo, m_context->allocationCallbacks, &m_context->instance), "Could not create instance");
}

void VulkanRenderer::createDevice(VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_context->physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_context->physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_context->graphicsFamilyIndex = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_context->physicalDevice, i, surface, &presentSupport);

        if (presentSupport) {
            m_context->presentFamilyIndex = i;
        }

        if (m_context->graphicsFamilyIndex >= 0 && m_context->presentFamilyIndex >= 0) break;
    }

    if (m_context->graphicsFamilyIndex < 0 || m_context->presentFamilyIndex < 0) {
        std::println("Device does not support graphics");
        exit(1);
    }

    auto isDeviceSuitable = [this, surface](Ref<VulkanContext> context) {
        VkPhysicalDevice physicalDevice = context->physicalDevice;

        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(m_context->physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(m_context->physicalDevice, &deviceFeatures);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        if (formatCount) {
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        if (presentModeCount) {
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        }

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && checkDeviceExtensionSupport(context) && formatCount &&
               presentModeCount;
    };

    if (!isDeviceSuitable(m_context)) {
        std::println("Device missing required features and is not suitable");
        exit(1);
    }

    float queuePriority = 1.0f;

    uint32_t queueFamilies[2] = { (uint32_t) m_context->graphicsFamilyIndex, (uint32_t) m_context->presentFamilyIndex };
    VkDeviceQueueCreateInfo deviceQueueCreateInfos[2] = {};

    for (uint32_t i = 0; i < 2; ++i) {
        deviceQueueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfos[i].queueFamilyIndex = queueFamilies[i];
        deviceQueueCreateInfos[i].queueCount = 1;
        deviceQueueCreateInfos[i].pQueuePriorities = &queuePriority;
    }

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    physicalDeviceFeatures.fillModeNonSolid = true;
    physicalDeviceFeatures.wideLines = true;
    physicalDeviceFeatures.multiViewport = true;
    physicalDeviceFeatures.samplerAnisotropy = true;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures{};
    dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &dynamicRenderingFeatures;
    deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos;
    deviceCreateInfo.queueCreateInfoCount = m_context->graphicsFamilyIndex == m_context->presentFamilyIndex ? 1 : 2;
    deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    deviceCreateInfo.enabledExtensionCount = requiredExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifndef DIST
    if (checkValidationLayerSupport()) {
        deviceCreateInfo.enabledLayerCount = validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        TK_ASSERT(false, "1 or more validation layers not supported by the system");
    }
#endif  // !DIST

    TK_ASSERT_VK_RESULT(
        vkCreateDevice(m_context->physicalDevice, &deviceCreateInfo, m_context->allocationCallbacks, &m_context->device), "Could not create device"
    );

    vkGetDeviceQueue(m_context->device, m_context->graphicsFamilyIndex, 0, &m_context->graphicsQueue);
    vkGetDeviceQueue(m_context->device, m_context->presentFamilyIndex, 0, &m_context->presentQueue);
}

#pragma region Utils

bool checkDeviceExtensionSupport(Ref<VulkanContext> context) {
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(context->physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(context->physicalDevice, nullptr, &extensionCount, deviceExtensions.data());

    uint32_t requiredExtentionCount = requiredExtensions.size();

    for (const auto& req : requiredExtensions) {
        std::string requiredExtension(req);
        for (const auto& deviceExtension : deviceExtensions) {
            if (std::string(deviceExtension.extensionName) == requiredExtension) {
                --requiredExtentionCount;
                break;
            }
        }
    }

    return requiredExtentionCount == 0;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    for (const char* requiredLayer : validationLayers) {
        bool layerFound = false;

        for (const auto& foundLayer : layers) {
            if (strcmp(requiredLayer, foundLayer.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) return false;
    }

    return true;
}

#pragma endregion Utils

}  // namespace Toki
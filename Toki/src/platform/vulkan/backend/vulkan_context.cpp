#include "tkpch.h"
#include "vulkan_context.h"
#include "vulkan_utils.h"
#include "vulkan_pipeline.h"
#include "core/assert.h"
#include "core/engine.h"

namespace Toki {

    VulkanContext::VulkanContext() {
        createInstance();
        createSurface();

        // TODO: Add some kind of selection for GPUs, currently only using first one found
        physicalDevice = enumeratePhysicalDevices().front();
        initQueueFamilyIndexes();
        initQueueFamilyProperties();

        createDevice();
    }

    VulkanContext::~VulkanContext() {
        vkDeviceWaitIdle(device);

        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    void VulkanContext::init() {
        createDescriptorPool();
        createCommandPool();
        createFrames();

        vkGetDeviceQueue(device, queueFamilyIndexes.graphicsQueueIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(device, queueFamilyIndexes.presentQueueIndex, 0, &presentQueue);

        swapchain = createRef<VulkanSwapchain>();
    }

    void VulkanContext::shutdown() {
        vkDeviceWaitIdle(device);

        swapchain.reset();

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyCommandPool(device, commandPool, nullptr);

        for (uint32_t i = 0; i < MAX_FRAMES; ++i) {
            vkDestroyFence(device, frames[i].renderFence, nullptr);
            vkDestroySemaphore(device, frames[i].renderSemaphore, nullptr);
            vkDestroySemaphore(device, frames[i].presentSemaphore, nullptr);
        }
    }

    void VulkanContext::createInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.engineVersion = 1;
        appInfo.pEngineName = "Toki";
        appInfo.pApplicationName = "Toki";
        appInfo.applicationVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_3;

        const char** glfwExtensions;
        uint32_t extensionCount = 0;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        auto result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &appInfo;
        instanceCreateInfo.enabledExtensionCount = extensionCount;
        instanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;

    #ifndef DIST
        TK_ASSERT(VulkanUtils::checkForValidationLayerSupport(), "Vulkan validation layers are not supported! Reinstalling SDK may help.");
        instanceCreateInfo.enabledLayerCount = VulkanUtils::validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = VulkanUtils::validationLayers.data();
    #endif

        TK_ASSERT_VK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), "Could not create Vulkan instance!");
    }

    void VulkanContext::createSurface() {
        TK_ASSERT_VK_RESULT(glfwCreateWindowSurface(instance, (GLFWwindow*) Engine::getWindow()->getHandle(), nullptr, &surface), "Could not create Vulkan window surface");
    }

    void VulkanContext::createDevice() {
        float queuePriority = 0.0f;
        VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndexes.graphicsQueueIndex;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.fillModeNonSolid = true;
        deviceFeatures.wideLines = true;
        deviceFeatures.multiViewport = true;
        deviceFeatures.samplerAnisotropy = true;

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.ppEnabledExtensionNames = VulkanUtils::deviceExtensions.data();
        deviceCreateInfo.enabledExtensionCount = VulkanUtils::deviceExtensions.size();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    #ifndef DIST
        TK_ASSERT(VulkanUtils::checkForValidationLayerSupport(), "Vulkan validation layers are not supported! Reinstalling SDK may help.");
        deviceCreateInfo.ppEnabledLayerNames = VulkanUtils::validationLayers.data();
        deviceCreateInfo.enabledLayerCount = VulkanUtils::validationLayers.size();
    #endif

        TK_ASSERT_VK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device), "Could not create Vulkan device");
    }

    void VulkanContext::initQueueFamilyIndexes() {
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

        TK_ASSERT(queueFamilyIndexes.presentQueueIndex < UINT32_MAX && queueFamilyIndexes.graphicsQueueIndex < UINT32_MAX, "Could not initialize queue family indexes!");
    }

    void VulkanContext::initQueueFamilyProperties() {
        uint32_t count{};
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
        queueFamilyProperties.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamilyProperties.data());
    }

    void VulkanContext::createDescriptorPool() {
        const uint32_t MAX_SETS = 100;

        DestriptorPoolSizes sizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
        };

        std::vector<VkDescriptorPoolSize> poolSizes(sizes.size());

        for (uint32_t i = 0; i < sizes.size(); ++i) {
            poolSizes[i].type = sizes[i].type;
            poolSizes[i].descriptorCount = sizes[i].size;
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.maxSets = MAX_SETS;

        TK_ASSERT_VK_RESULT(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool), "Could not create VkDescriptorPool");
    }

    void VulkanContext::createCommandPool() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndexes.graphicsQueueIndex;

        TK_ASSERT_VK_RESULT(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool), "Could not create command pool");
    }

    void VulkanContext::createFrames() {
        const auto [graphicsQueueIndex, presentQueueIndex] = queueFamilyIndexes;

        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        for (uint32_t i = 0; i < VulkanContext::MAX_FRAMES; ++i) {
            TK_ASSERT_VK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &frames[i].commandBuffer), "");
            TK_ASSERT_VK_RESULT(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence), "");
            TK_ASSERT_VK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].presentSemaphore), "");
            TK_ASSERT_VK_RESULT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore), "");
        }
    }

    std::vector<VkPhysicalDevice> VulkanContext::enumeratePhysicalDevices() {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

        return physicalDevices;
    }

}
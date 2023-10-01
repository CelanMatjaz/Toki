#include "tkpch.h"
#include "vulkan_utils.h"
#include "algorithm"
#include "platform/vulkan/vulkan_renderer.h"
#include "core/assert.h"
#include "core/engine.h"

namespace Toki {

    bool VulkanUtils::checkForValidationLayerSupport() {
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

    VkExtent2D VulkanUtils::getExtent() {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanRenderer::physicalDevice(), VulkanRenderer::surface(), &capabilities);

        uint32_t width = Engine::getWindow()->getWidth();
        uint32_t height = Engine::getWindow()->getHeight();

        if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) {
            VkExtent2D extent;
            extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return extent;
        }

        return capabilities.currentExtent;
    }

    uint32_t VulkanUtils::getImageCount(uint32_t maxImageCount) {
        VkSurfaceCapabilitiesKHR capabilities;
        auto t = VulkanRenderer::physicalDevice();
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanRenderer::physicalDevice(), VulkanRenderer::surface(), &capabilities);

        if (maxImageCount > 0 && maxImageCount <= capabilities.maxImageCount) {
            return maxImageCount;
        }
        return capabilities.minImageCount;
    }

    VkSurfaceTransformFlagBitsKHR VulkanUtils::getPreTransform() {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanRenderer::physicalDevice(), VulkanRenderer::surface(), &capabilities);

        return (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
            ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
            : capabilities.currentTransform;
    }

    VkCompositeAlphaFlagBitsKHR VulkanUtils::getCompositeAlpha() {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanRenderer::physicalDevice(), VulkanRenderer::surface(), &capabilities);

        return (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
            : (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) ? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
            : (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
            : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    }

    VkImageTiling VulkanUtils::findTiling(VkFormat format) {
        VkPhysicalDevice physicalDevice = VulkanRenderer::physicalDevice();

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

        if (formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return VK_IMAGE_TILING_LINEAR;
        }
        else if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            return VK_IMAGE_TILING_OPTIMAL;
        }

        TK_ASSERT(false, "Format not supported");
        throw new std::runtime_error("");

    }

    VkFormat VulkanUtils::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlagBits features) {
        VkPhysicalDevice physicalDevice = VulkanRenderer::physicalDevice();

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

        TK_ASSERT(false, "Failed to find suitable format");
    }

    VkSurfaceFormatKHR VulkanUtils::findSurfaceFormat() {
        VkPhysicalDevice physicalDevice = VulkanRenderer::physicalDevice();
        VkSurfaceKHR surface = VulkanRenderer::surface();

        uint32_t surfaceFormatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, formats.data());

        for (const auto& availableFormat : formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return formats[0];
    }

    uint32_t VulkanUtils::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) {
        VkPhysicalDevice physicalDevice = VulkanRenderer::physicalDevice();

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

    VkFormat VulkanUtils::findDepthFormat() {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    VkFormat VulkanUtils::mapFormat(Format format) {
        switch (format) {
            case Format::RGBA8: return VK_FORMAT_B8G8R8A8_SRGB;
            case Format::R32: return VK_FORMAT_R32_SFLOAT;
            case Format::R32G32i: return VK_FORMAT_R32G32_SINT;
            case Format::Depth: return findDepthFormat();
            case Format::DepthStencil: return VK_FORMAT_D24_UNORM_S8_UINT;
        }

        return VK_FORMAT_UNDEFINED;
    }

    VkSampleCountFlagBits VulkanUtils::mapSamples(Samples samples) {
        switch (samples) {
            case Samples::Sample1: return VK_SAMPLE_COUNT_1_BIT;
            case Samples::Sample2: return VK_SAMPLE_COUNT_2_BIT;
            case Samples::Sample4: return VK_SAMPLE_COUNT_4_BIT;
            case Samples::Sample8: return VK_SAMPLE_COUNT_8_BIT;
            case Samples::Sample16: return VK_SAMPLE_COUNT_16_BIT;
            case Samples::Sample32: return VK_SAMPLE_COUNT_32_BIT;
            case Samples::Sample64: return VK_SAMPLE_COUNT_64_BIT;
        }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    VkAttachmentLoadOp VulkanUtils::mapLoadOp(AttachmentLoadOp loadOp) {
        switch (loadOp) {
            case AttachmentLoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            case AttachmentLoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
            case AttachmentLoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
        }

        return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    }

    VkAttachmentStoreOp VulkanUtils::mapStoreOp(AttachmentStoreOp storeOp) {
        switch (storeOp) {
            case AttachmentStoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
            case AttachmentStoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
        }

        return VK_ATTACHMENT_STORE_OP_DONT_CARE;
    }

    VkFormat VulkanUtils::mapVertexFormat(VertexFormat format) {
        switch (format) {
            case VertexFormat::Float1: return VK_FORMAT_R32_SFLOAT;
            case VertexFormat::Float2: return VK_FORMAT_R32G32_SFLOAT;
            case VertexFormat::Float3: return VK_FORMAT_R32G32B32_SFLOAT;
            case VertexFormat::Float4: return VK_FORMAT_R32G32B32A32_SFLOAT;
            case VertexFormat::Int1: return VK_FORMAT_R32_SINT;
            case VertexFormat::Int2: return VK_FORMAT_R32G32_SINT;
            case VertexFormat::Int3: return VK_FORMAT_R32G32B32_SINT;
            case VertexFormat::Int4: return VK_FORMAT_R32G32B32A32_SINT;
        }

        return VK_FORMAT_UNDEFINED;
    }

    VkPrimitiveTopology VulkanUtils::mapTopology(PrimitiveTopology topology) {
        switch (topology) {
            case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case PrimitiveTopology::TriangleFan: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        }
    }

    VkFrontFace VulkanUtils::mapFrontFace(FrontFace frontFace) {
        switch (frontFace) {
            case FrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            case FrontFace::Clockwise: return VK_FRONT_FACE_CLOCKWISE;
        }
    }

    VkCullModeFlags VulkanUtils::mapCullMode(CullMode cullMode) {
        switch (cullMode) {
            case CullMode::None: return VK_CULL_MODE_NONE;
            case CullMode::Front: return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back: return VK_CULL_MODE_BACK_BIT;
            case CullMode::FrontAndBack: return VK_CULL_MODE_FRONT_AND_BACK;
        }
    }

    VkCompareOp VulkanUtils::mapCompareOp(CompareOp compareOp) {
        switch (compareOp) {
            case CompareOp::Never: return VK_COMPARE_OP_NEVER;
            case CompareOp::Less: return VK_COMPARE_OP_LESS;
            case CompareOp::Equal: return VK_COMPARE_OP_EQUAL;
            case CompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOp::Greater: return VK_COMPARE_OP_GREATER;
            case CompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOp::Always: return VK_COMPARE_OP_ALWAYS;
        }
    }

    VkStencilOp VulkanUtils::mapStencilOpState(StencilOp stencilOp) {
        switch (stencilOp) {
            case StencilOp::Keep: return VK_STENCIL_OP_KEEP;
            case StencilOp::Zero: return VK_STENCIL_OP_ZERO;
            case StencilOp::Replace: return VK_STENCIL_OP_REPLACE;
            case StencilOp::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            case StencilOp::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            case StencilOp::Invert: return VK_STENCIL_OP_INVERT;
            case StencilOp::IncrementAndWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
            case StencilOp::DecrementAndWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        }
    }

    VkStencilOpState VulkanUtils::mapStencilOpState(StencilOpState stencilOpState) {
        VkStencilOpState state{};
        state.failOp = mapStencilOpState(stencilOpState.failOp);
        state.passOp = mapStencilOpState(stencilOpState.passOp);
        state.depthFailOp = mapStencilOpState(stencilOpState.depthFailOp);
        state.compareOp = mapCompareOp(stencilOpState.compareOp);
        state.compareMask = stencilOpState.compareMask;
        state.writeMask = stencilOpState.writeMask;
        state.reference = stencilOpState.reference;
        return state;
    }

}
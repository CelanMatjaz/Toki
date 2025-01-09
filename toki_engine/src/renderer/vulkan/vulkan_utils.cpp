#include "renderer/vulkan/vulkan_utils.h"

#include <vulkan/vulkan.h>

#include <set>
#include <vector>

#include "core/assert.h"
#include "core/logging.h"
#include "renderer/renderer_types.h"
#include "renderer/vulkan/vulkan_context.h"
#include "shaderc/shaderc.hpp"

namespace toki {

#ifndef TK_DIST

bool check_validation_layer_support() {
    uint32_t layer_count{};
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

    for (const char* required_layer : validation_layers) {
        bool layer_found = false;

        for (const auto& found_layer : layers) {
            if (strcmp(required_layer, found_layer.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) return false;
    }

    return true;
}

#endif

bool check_for_extensions(VkPhysicalDevice physical_device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    TK_ASSERT(extension_count > 0, "No extensions available on device");
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(vulkan_extensions.begin(), vulkan_extensions.end());

    for (const auto& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

bool is_device_suitable(VkPhysicalDevice physical_device) {
    return check_for_extensions(physical_device);
}

QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    u32 queue_family_count{};
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

    QueueFamilyIndices indices{};
    for (int i = 0; i < queue_families.size(); i++) {
        VkBool32 supports_present = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present);
        if (supports_present && indices.present == -1) {
            indices.present = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && indices.graphics == -1) {
            indices.graphics = i;
        }

        if (queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT && indices.transfer == -1) {
            indices.transfer = i;
        }
    }

    TK_ASSERT(indices.present > -1 && indices.graphics > -1 && indices.transfer > -1, "GPU does not support all required queues");

    return indices;
}

u32 find_memory_type(VkPhysicalDevice physical_device, u32 type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
    for (u32 i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    TK_ASSERT(false, "No memory type found");
    std::unreachable();
}

VkShaderModule create_shader_module(Ref<RendererContext> ctx, std::vector<u32>& binary) {
    VkShaderModuleCreateInfo shader_module_create_info{};
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = binary.size() * 4;
    shader_module_create_info.pCode = binary.data();

    VkShaderModule shader_module{};
    VK_CHECK(vkCreateShaderModule(ctx->device, &shader_module_create_info, ctx->allocation_callbacks, &shader_module), "Could not create shader module");

    return shader_module;
}

std::vector<u32> compile_shader(ShaderStage stage, std::string& source) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
    options.SetSourceLanguage(shaderc_source_language::shaderc_source_language_glsl);

#ifdef TK_NDEBUG
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
#else
    options.SetOptimizationLevel(shaderc_optimization_level_zero);
#endif

    shaderc_shader_kind shader_kind;

    switch (stage) {
        case ShaderStage::VERTEX:
            shader_kind = shaderc_shader_kind::shaderc_glsl_vertex_shader;
            break;
        case ShaderStage::FRAGMENT:
            shader_kind = shaderc_shader_kind::shaderc_glsl_fragment_shader;
            break;
        default:
            std::unreachable();
    }

    shaderc::SpvCompilationResult spirv_module = compiler.CompileGlslToSpv(source.data(), shader_kind, ".", options);
    if (spirv_module.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success) {
        TK_LOG_ERROR("ERROR MESSAGE:\n{}\n\nCOMPILATION STATUS: {}\nSOURCE: {}", spirv_module.GetErrorMessage(), (int) spirv_module.GetCompilationStatus(), source);
    }
    TK_ASSERT(spirv_module.GetCompilationStatus() == shaderc_compilation_status::shaderc_compilation_status_success, "Shader compilation error");

    TK_ASSERT(spirv_module.end() - spirv_module.begin() > 0, "");
    return std::vector<u32>{ spirv_module.begin(), spirv_module.end() };
}

VkFormat map_format(ColorFormat format) {
    switch (format) {
        case ColorFormat::R8:
            return VK_FORMAT_R8_SRGB;
        case ColorFormat::RGBA8:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ColorFormat::DEPTH:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case ColorFormat::STENCIL:
            return VK_FORMAT_S8_UINT;
        case ColorFormat::DEPTH_STENCIL:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        default:
            std::unreachable();
    }
}

VkAttachmentLoadOp map_attachment_load_op(RenderTargetLoadOp op) {
    switch (op) {
        case toki::RenderTargetLoadOp::LOAD:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case RenderTargetLoadOp::CLEAR:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case RenderTargetLoadOp::DONT_CARE:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:
            std::unreachable();
    }
}

VkAttachmentStoreOp map_attachment_store_op(RenderTargetStoreOp op) {
    switch (op) {
        case RenderTargetStoreOp::STORE:
            return VK_ATTACHMENT_STORE_OP_STORE;
        case RenderTargetStoreOp::DONT_CARE:
            return VK_ATTACHMENT_STORE_OP_DONT_CARE;
        default:
            std::unreachable();
    }
}

VkBufferUsageFlags map_buffer_type(BufferType type) {
    switch (type) {
        case BufferType::VERTEX:
            return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        case BufferType::INDEX:
            return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        default:
            std::unreachable();
    }
}

VkMemoryPropertyFlags map_buffer_memory_properties(BufferUsage usage) {
    VkMemoryPropertyFlags flags = 0;
    switch (usage) {
        case BufferUsage::STATIC:
            flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case toki::BufferUsage::DYNAMIC:
            flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
        default:
            std::unreachable();
    }

    return flags;
}

}  // namespace toki

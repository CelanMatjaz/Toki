#pragma once

#include <vulkan/vulkan.h>

#include "renderer/vulkan_types.h"
#include "toki/renderer/shader.h"

namespace Toki {

class VulkanRenderer;

class VulkanGraphicsPipeline : public Shader {
    friend VulkanRenderer;

public:
    VulkanGraphicsPipeline(const ShaderConfig& config);
    ~VulkanGraphicsPipeline();

    VkPipeline getPipeline() const;

private:
    inline static Ref<VulkanContext> s_context;

    void create();
    void destroy();

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
};

}  // namespace Toki

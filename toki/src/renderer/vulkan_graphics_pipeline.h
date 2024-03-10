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
    VkPipelineLayout getPipelineLayout() const;
    uint32_t getPushConstantStageFlags() const;
    const std::vector<VkDescriptorSet>& getDestriptorSets() const;

    virtual void setUniforms(std::vector<Ref<UniformBuffer>> uniforms) override;

private:
    inline static Ref<VulkanContext> s_context;

    void create();
    void destroy();

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    uint32_t m_pushConstantStageFlags = 0;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
};

}  // namespace Toki

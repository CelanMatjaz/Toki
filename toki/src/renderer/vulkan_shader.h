#pragma once

#include <vulkan/vulkan.h>

#include "renderer/pipeline/vulkan_pipeline.h"
#include "renderer/vulkan_types.h"
#include "toki/renderer/shader.h"

namespace Toki {

class VulkanRenderer;

class VulkanShader : public Shader {
    friend VulkanRenderer;

public:
    VulkanShader(const ShaderConfig& config);
    ~VulkanShader();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    uint32_t getPushConstantStageFlags() const;
    const std::vector<VkDescriptorSet>& getDestriptorSets() const;

    virtual void setUniforms(std::vector<Uniform> uniforms) override;

private:
    inline static Ref<VulkanContext> s_context;

    void create();
    void destroy();

    Ref<VulkanPipeline> m_pipeline;
};

}  // namespace Toki

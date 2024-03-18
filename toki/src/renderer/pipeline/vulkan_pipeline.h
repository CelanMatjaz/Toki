#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include "renderer/vulkan_types.h"
#include "toki/core/core.h"
#include "toki/renderer/renderer_types.h"
#include "toki/renderer/shader.h"

namespace Toki {

struct ShaderSource {
    ShaderStage shaderStage;
    std::string source;
};

using ShaderSources = std::vector<ShaderSource>;

struct ShaderBinary {
    ShaderStage shaderStage;
    std::vector<uint32_t> spirv;
};

using ShaderBinaries = std::vector<ShaderBinary>;

class VulkanRenderer;

class VulkanPipeline {
    friend VulkanRenderer;

public:
    static Ref<VulkanPipeline> create(const ShaderBinaries& sources, const GraphicsShaderOptions& options);
    static Ref<VulkanPipeline> create(const ShaderBinaries& sources, const ComputeShaderOptions& options);

    VulkanPipeline() = default;
    virtual ~VulkanPipeline();

    VkPipeline getPipeline() const;
    VkPipelineLayout getPipelineLayout() const;
    VkShaderStageFlagBits getPushConstantShaderStageFlags() const;
    const std::vector<VkPushConstantRange>& getPushConstantRanges() const;
    const std::vector<VkDescriptorSet>& getDestriptorSets() const;

    virtual VkPipelineBindPoint getBindPoint() const = 0;
    virtual VkPipeline create(const ShaderBinaries& binaries) = 0;
    void destroy();

protected:
    inline static Ref<VulkanContext> s_context;

    void createLayoutFromBinaries(const ShaderBinaries& binaries);
    void allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& layouts);
    void createShaderStageCreateInfos(const ShaderBinaries& binaries, std::vector<VkPipelineShaderStageCreateInfo>& shaderStageCreateInfos);

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    std::vector<VkDescriptorSet> m_descriptorSets;
    uint32_t m_pushConstantStageFlags = 0;
};

}  // namespace Toki

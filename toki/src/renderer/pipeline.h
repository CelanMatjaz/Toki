#pragma once

#include <vulkan/vulkan.h>

#include "renderer/vulkan_types.h"
#include "toki/resources/resource_types.h"

namespace Toki {

using ShaderPipelineBinaries = std::unordered_map<ShaderStage, std::vector<uint32_t>>;
using SetBindings = std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>;

struct Pipeline {
    Pipeline() = delete;
    Pipeline(const ShaderConfig& config);
    ~Pipeline();

    operator VkPipeline() const { return m_pipeline; }

    void create(const ShaderPipelineBinaries& binaries);
    void destroy();

    void createPipelineLayout(const ShaderPipelineBinaries& binaries);

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    AttachmentFormatHash m_renderPassHandle;
    uint32_t m_pushConstantStageFlags = 0;

    ShaderConfig m_config;

    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
    std::vector<VkDescriptorSet> m_descriptorSets;

private:
    void allocateDescriptorSets(std::vector<VkDescriptorSetLayout>& layouts);
};

}  // namespace Toki

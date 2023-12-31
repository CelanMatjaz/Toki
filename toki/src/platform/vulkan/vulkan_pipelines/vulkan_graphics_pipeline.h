#pragma once

#include "vulkan/vulkan_core.h"
#include "vulkan/vulkan_pipelines/vulkan_pipeline.h"
#include "vulkan/vulkan_types.h"

namespace Toki {

struct VulkanGraphicsPipelineConfig : public PipelineConfig {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    bool primitiveRestartEnable = false;
    PolygonMode polygonMode;
    CullMode cullMode = CullMode::None;
    FrontFace frontFace = FrontFace::CounterClockwise;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    uint32_t subpass = 0;
};

class GraphicsPipeline : public VulkanPipeline {
public:
    GraphicsPipeline() = delete;
    GraphicsPipeline(VulkanContext* context, const VulkanGraphicsPipelineConfig& config);
    ~GraphicsPipeline();

    void recreate();

private:
    virtual void create() override;
    virtual void destroy() override;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VulkanGraphicsPipelineConfig config;
};

}  // namespace Toki

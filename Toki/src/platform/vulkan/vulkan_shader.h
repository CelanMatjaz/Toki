#pragma once

#include "renderer/shader.h"
#include "vulkan/vulkan.h"
#include "platform/vulkan/backend/vulkan_pipeline.h"
#include "platform/vulkan/backend/vulkan_render_pass.h"
#include "spirv_cross/spirv_reflect.hpp"
#include "shaderc/shaderc.hpp"
#include "unordered_map"

namespace Toki {

    class VulkanShader : public Shader {
    public:
        VulkanShader(const ShaderConfig& config);
        ~VulkanShader() override;
        virtual void bind() override;
        virtual void reload() override;

        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }
        VkDescriptorSet getDescriptorSet(ShaderStage stage, uint32_t set) { return descriptorSets[std::to_underlying(stage)][set]; }
        std::vector<VkDescriptorSet> getDescriptorSets(ShaderStage stage);

        VkPipelineBindPoint getPipelineBindPoint();

        static VkShaderStageFlagBits mapShaderStage(ShaderStage stage);

    private:
        Ref<Pipeline> pipeline;

        void initGraphics();

        VkShaderModule reflect(ShaderStage stage, std::string_view shaderCode);

        static VkShaderStageFlagBits getVulkanShaderStage(ShaderStage stage); // TODO: remove
        static shaderc_shader_kind getShadercShaderKind(ShaderStage stage);

        static std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> getDescriptorSetBindings(ShaderStage stage, spirv_cross::Compiler& compiler, VkDescriptorType descriptorType);
        static std::vector<VkPushConstantRange> getPushConstants(ShaderStage stage, spirv_cross::Compiler& compiler);

        static std::vector<VkVertexInputBindingDescription> mapBindingDescriptions(std::vector<VertexBindingDescription> bindingDescriptions);
        static std::vector<VkVertexInputAttributeDescription> mapAttributeDescriptions(std::vector<VertexAttributeDescription> attributeDescriptions);

        std::vector<VkPushConstantRange> pushConstants;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkDescriptorSet> descriptorSets[std::to_underlying(ShaderStage::SHADER_STAGE_MAX_ENUM)];
        VkPipelineLayout pipelineLayout;
        VkPipelineBindPoint bindPoint;
        Ref<VulkanRenderPass> renderPass;
    };

}
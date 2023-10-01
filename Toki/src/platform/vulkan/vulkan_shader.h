#pragma once

#include "renderer/shader.h"
#include "vulkan/vulkan.h"
#include "platform/vulkan/vulkan_render_pass.h"
#include "platform/vulkan/backend/vulkan_descriptor_set.h"
#include "platform/vulkan/backend/vulkan_pipeline.h"
#include "unordered_map"
#include "spirv_cross/spirv_reflect.hpp"
#include "shaderc/shaderc.hpp"
#include "renderer/buffer.h"
#include "renderer/texture.h"

namespace Toki {

    class VulkanShader : public Shader {
    public:
        VulkanShader(const ShaderConfig& config);
        ~VulkanShader() override;
        virtual void bind() override;
        virtual void reload() override;

        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }
        void setUniform(Ref<UniformBuffer> uniformBuffer, uint32_t binding, uint32_t set = 0, uint32_t index = 0);
        void setTexture(Ref<Texture> texture, uint32_t binding, uint32_t set = 0, uint32_t index = 0);

        VkDescriptorSet getSet(uint32_t set);
        const std::vector<VkDescriptorSet>& getSets() const { return sets; };
        VkShaderStageFlagBits getContantStageFlags() { return (VkShaderStageFlagBits) contantStageFlags; }

        VkPipelineBindPoint getPipelineBindPoint();

        static VkShaderStageFlagBits mapShaderStage(ShaderStage stage);

    private:
        Ref<Pipeline> pipeline;

        void initGraphics();

        VkShaderModule reflect(ShaderStage stage, const std::vector<uint32_t> spirv);

        static shaderc_shader_kind getShadercShaderKind(ShaderStage stage);

        void getDescriptorSetBindings(ShaderStage stage, spirv_cross::Compiler& compiler, VkDescriptorType descriptorType);
        void getPushConstants(ShaderStage stage, spirv_cross::Compiler& compiler);

        static std::vector<VkVertexInputBindingDescription> mapBindingDescriptions(std::vector<VertexBindingDescription> bindingDescriptions);
        static std::vector<VkVertexInputAttributeDescription> mapAttributeDescriptions(std::vector<VertexAttributeDescription> attributeDescriptions);

        std::vector<VkPushConstantRange> pushConstants;
        std::unordered_map<uint32_t, VulkanDescriptorSet> descriptorSets;
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> setLayoutBindings;
        std::vector<VkDescriptorSet> sets;
        VkPipelineLayout pipelineLayout;
        VkPipelineBindPoint bindPoint;
        Ref<VulkanRenderPass> renderPass;

        uint32_t contantStageFlags = 0;
    };

}
#include "tkpch.h"
#include "vulkan_descriptor_set.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "core/assert.h"

namespace Toki {

    // TODO: 
    // Use unordered_map instead of vector to better support descriptor bindings
    // Not all bindings are necessarily in order or have sequential binding indecies
    // Shaders are currently forced to have sequential binding indecies
    VulkanDescriptorSet::VulkanDescriptorSet(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
        for (uint32_t i = 0; i < bindings.size(); ++i) {
            switch (bindings[i].descriptorType) {
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    setBindings[i] = { DescriptorSetBindingType::Uniform, bindings[i].descriptorCount };
                    break;
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                    setBindings[i] = { DescriptorSetBindingType::Sampler2D, bindings[i].descriptorCount };
                    break;
                default:
                    TK_ASSERT(false, std::format("Descriptor type {} is not supported", (int) bindings[i].descriptorType));
            }
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.pBindings = bindings.data();
        layoutInfo.bindingCount = bindings.size();

        TK_ASSERT_VK_RESULT(vkCreateDescriptorSetLayout(VulkanRenderer::device(), &layoutInfo, nullptr, &layout), "Could not create descriptor set layout for descriptor set");

        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocateInfo.descriptorPool = VulkanRenderer::descriptorPool();
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &layout;

        TK_ASSERT_VK_RESULT(vkAllocateDescriptorSets(VulkanRenderer::device(), &descriptorSetAllocateInfo, &set), "Could not allocate descriptor sets");

        initBindings();
    }

    VulkanDescriptorSet::~VulkanDescriptorSet() {
        vkDestroyDescriptorSetLayout(VulkanRenderer::device(), layout, nullptr);
    }

    void VulkanDescriptorSet::setUniformBuffer(VulkanUniformBuffer* uniformBuffer, uint32_t binding, uint32_t index) {
        TK_ASSERT(setBindings[binding].type == DescriptorSetBindingType::Uniform, std::format("Trying to set a descriptor Uniform to some other type: {}", std::to_underlying(setBindings[binding].type)));
        TK_ASSERT(setBindings.contains(binding), std::format("Binding {} does not exist on shader", binding));
        TK_ASSERT(setBindings[binding].size > index, std::format("Index out of bounds. Trying to set array index {} on a binding of size {}", index, setBindings[binding].size));

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffer->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = uniformBuffer->getSize();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = index;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.descriptorCount = 1;

        vkUpdateDescriptorSets(VulkanRenderer::device(), 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanDescriptorSet::setTexture(VulkanTexture* texture, uint32_t binding, uint32_t index) {
        TK_ASSERT(setBindings[binding].type == DescriptorSetBindingType::Sampler2D, std::format("Trying to set a descriptor Sampler2D to some other type: {}", std::to_underlying(setBindings[binding].type)));
        TK_ASSERT(setBindings.contains(binding), std::format("Binding {} does not exist on shader", binding));
        TK_ASSERT(setBindings[binding].size > index, std::format("Index out of bounds. Trying to set array index {} on a binding of size {}", index, setBindings[binding].size));

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = texture->getHandle()->getView();
        imageInfo.sampler = VulkanRenderer::sampler();

        std::vector<VkDescriptorImageInfo > asd(setBindings[binding].size, imageInfo);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = index;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.pImageInfo = &imageInfo;
        descriptorWrite.descriptorCount = 1;

        vkUpdateDescriptorSets(VulkanRenderer::device(), 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanDescriptorSet::initBindings() {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = set;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        for (const auto& [bindingKey, binding] : setBindings) {
            switch (binding.type) {
                case DescriptorSetBindingType::Uniform: {
                    VkDescriptorBufferInfo bufferInfo{};
                    bufferInfo.buffer = VulkanRenderer::defaultUniform()->getBuffer();
                    bufferInfo.offset = 0;
                    bufferInfo.range = 1;

                    std::vector<VkDescriptorBufferInfo> bufferInfos(binding.size, bufferInfo);

                    descriptorWrite.dstBinding = bindingKey;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    descriptorWrite.pBufferInfo = bufferInfos.data();
                    descriptorWrite.descriptorCount = bufferInfos.size();

                    descriptorWrites.push_back(descriptorWrite);

                    vkUpdateDescriptorSets(VulkanRenderer::device(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

                    break;
                }
                case DescriptorSetBindingType::Sampler2D: {
                    VkDescriptorImageInfo imageInfo{};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = VulkanRenderer::defaultTexture()->getView();
                    imageInfo.sampler = VulkanRenderer::sampler();

                    std::vector<VkDescriptorImageInfo> imageInfos(binding.size, imageInfo);

                    descriptorWrite.dstBinding = bindingKey;
                    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    descriptorWrite.pImageInfo = imageInfos.data();
                    descriptorWrite.descriptorCount = imageInfos.size();

                    descriptorWrites.push_back(descriptorWrite);

                    vkUpdateDescriptorSets(VulkanRenderer::device(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

                    break;
                }
            }
        }
    }

}
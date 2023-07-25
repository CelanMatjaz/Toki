#include "tkpch.h"
#include "infos.h"

namespace Toki {

    namespace Infos {

        namespace Pipeline {

            VkPipelineShaderStageCreateInfo shaderStageCreateInfo(const VkShaderStageFlagBits& stage, const VkShaderModule& shaderModule, const char* name) {
                VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
                shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStageCreateInfo.stage = stage;
                shaderStageCreateInfo.module = shaderModule;
                shaderStageCreateInfo.pName = name;
                return shaderStageCreateInfo;
            }

            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& inputBindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& inputAttributeDescriptions) {
                VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
                vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputStateCreateInfo.pVertexBindingDescriptions = inputBindingDescriptions.data();
                vertexInputStateCreateInfo.vertexBindingDescriptionCount = inputBindingDescriptions.size();
                vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data();
                vertexInputStateCreateInfo.vertexAttributeDescriptionCount = inputAttributeDescriptions.size();
                return vertexInputStateCreateInfo;
            }

            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo(const VkPrimitiveTopology& topology, const VkBool32& primitiveRestartEnable) {
                VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
                inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssemblyStateCreateInfo.topology = topology;
                inputAssemblyStateCreateInfo.primitiveRestartEnable = primitiveRestartEnable;
                return inputAssemblyStateCreateInfo;
            }

            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo(const VkPolygonMode& polygonMode, const VkFrontFace& frontFace, const VkCullModeFlags& cullMode) {
                VkPipelineRasterizationStateCreateInfo rasterizerCreateInto{};
                rasterizerCreateInto.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizerCreateInto.polygonMode = polygonMode;
                rasterizerCreateInto.frontFace = frontFace;
                rasterizerCreateInto.cullMode = cullMode;
                return rasterizerCreateInto;
            }

            VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo(const VkBool32& depthTest, const VkBool32& depthWrite, const VkCompareOp& compareOp) {
                VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
                depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                depthStencilStateCreateInfo.depthTestEnable = depthTest;
                depthStencilStateCreateInfo.depthWriteEnable = depthWrite;
                depthStencilStateCreateInfo.depthCompareOp = compareOp;
                depthStencilStateCreateInfo.minDepthBounds = 0.0f;
                depthStencilStateCreateInfo.maxDepthBounds = 1.0f;
                return depthStencilStateCreateInfo;
            }

            VkPipelineColorBlendAttachmentState colorBlendAttachmentState(const VkBool32& blendEnable, const VkColorComponentFlags& writeMask) {
                VkPipelineColorBlendAttachmentState colorBlendAttachment{};
                colorBlendAttachment.blendEnable = blendEnable;
                colorBlendAttachment.colorWriteMask = writeMask;
                return colorBlendAttachment;
            }

            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
                VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
                colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
                colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
                colorBlendStateCreateInfo.attachmentCount = attachments.size();
                colorBlendStateCreateInfo.pAttachments = attachments.data();
                return colorBlendStateCreateInfo;
            }

            VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo(const std::vector<VkDynamicState>& states) {
                VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
                dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicStateCreateInfo.dynamicStateCount = states.size();
                dynamicStateCreateInfo.pDynamicStates = states.data();
                return dynamicStateCreateInfo;
            }

        }

        namespace RenderPass {

            VkAttachmentDescription colorAttachmentDescription(const VkFormat& format) {
                VkAttachmentDescription colorAttachment{};
                colorAttachment.format = format;
                colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                return colorAttachment;
            }

            VkAttachmentDescription depthAttachmentDescription(const VkFormat& format) {
                VkAttachmentDescription depthAttachment{};
                depthAttachment.format = format;
                depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
                depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                return depthAttachment;
            }

            VkSubpassDescription subpassDescription(const std::vector<VkAttachmentReference>& colorAttachmentReferences, const VkAttachmentReference& depthReference) {
                VkSubpassDescription subpass{};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount = colorAttachmentReferences.size();
                subpass.pColorAttachments = colorAttachmentReferences.data();
                subpass.pDepthStencilAttachment = &depthReference;
                return subpass;
            }

            VkSubpassDependency colorSubpassDependency() {
                VkSubpassDependency colorDependency{};
                colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                colorDependency.dstSubpass = 0;
                colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                colorDependency.srcAccessMask = 0;
                colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                return colorDependency;
            }

            VkSubpassDependency depthSubpassDependency() {
                VkSubpassDependency depthDependency{};
                depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
                depthDependency.dstSubpass = 0;
                depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                depthDependency.srcAccessMask = 0;
                depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                return depthDependency;
            }

            VkRenderPassCreateInfo renderPassCreateInfo(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDependency>& dependencies, const std::vector<VkSubpassDescription>& subpasses) {
                VkRenderPassCreateInfo renderPassCreateInfo{};
                renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassCreateInfo.attachmentCount = attachments.size();
                renderPassCreateInfo.pAttachments = attachments.data();
                renderPassCreateInfo.dependencyCount = dependencies.size();
                renderPassCreateInfo.pDependencies = dependencies.data();
                renderPassCreateInfo.subpassCount = subpasses.size();
                renderPassCreateInfo.pSubpasses = subpasses.data();
                return renderPassCreateInfo;
            }

            VkRenderPassBeginInfo renderPassBeginInfo(const VkRenderPass& renderPass, const VkExtent2D& extent, const VkFramebuffer& framebuffer, const std::vector<VkClearValue>& clearValues) {
                VkRenderPassBeginInfo renderPassBeginInfo{};
                renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassBeginInfo.renderPass = renderPass;
                renderPassBeginInfo.renderArea.offset.x = 0;
                renderPassBeginInfo.renderArea.offset.y = 0;
                renderPassBeginInfo.renderArea.extent = extent;
                renderPassBeginInfo.framebuffer = framebuffer;
                renderPassBeginInfo.clearValueCount = clearValues.size();
                renderPassBeginInfo.pClearValues = clearValues.data();
                return renderPassBeginInfo;
            }

        }

        namespace Renderer {

            VkFenceCreateInfo fenceCreateInfo(const VkFenceCreateFlags& flags) {
                VkFenceCreateInfo fenceCreateInfo{};
                fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceCreateInfo.flags = flags;
                return fenceCreateInfo;
            }

            VkSemaphoreCreateInfo semaphoreCreateInfo() {
                VkSemaphoreCreateInfo semaphoreCreateInfo{};
                semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
                return semaphoreCreateInfo;
            }

            VkCommandPoolCreateInfo commandPoolCreateInfo(VkCommandPoolCreateFlags flags, const uint32_t& queueFamilyIndex) {
                VkCommandPoolCreateInfo commandPoolCreateInfo{};
                commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                commandPoolCreateInfo.flags = flags;
                commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
                return commandPoolCreateInfo;
            }

            VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool commandPool, uint32_t count, VkCommandBufferLevel level) {
                VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
                commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                commandBufferAllocateInfo.commandPool = commandPool;
                commandBufferAllocateInfo.commandBufferCount = count;
                commandBufferAllocateInfo.level = level;
                return commandBufferAllocateInfo;
            }

            VkSwapchainCreateInfoKHR swapchainCreateInfo() {
                VkSwapchainCreateInfoKHR swapchainCreateInfo{};
                swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
                return swapchainCreateInfo;
            }

            VkFramebufferCreateInfo framebufferCreateInfo(const VkRenderPass& renderPass, const uint32_t& width, const uint32_t& height, const uint32_t& layers, const std::vector<VkImageView>& attachments) {
                VkFramebufferCreateInfo framebufferCreateInfo{};
                framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferCreateInfo.renderPass = renderPass;
                framebufferCreateInfo.width = width;
                framebufferCreateInfo.height = height;
                framebufferCreateInfo.layers = layers;
                framebufferCreateInfo.attachmentCount = attachments.size();
                framebufferCreateInfo.pAttachments = attachments.data();
                return framebufferCreateInfo;
            }

            VkCommandBufferAllocateInfo commandBufferAllocateInfo(const VkCommandPool& commandPool, const VkCommandBufferLevel& level, const uint32_t& count) {
                VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
                commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                commandBufferAllocateInfo.commandPool = commandPool;
                commandBufferAllocateInfo.level = level;
                commandBufferAllocateInfo.commandBufferCount = count;
                return commandBufferAllocateInfo;
            }

            VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlagBits flags) {
                VkCommandBufferBeginInfo commandBufferBeginInfo{};
                commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                commandBufferBeginInfo.flags = flags;
                return commandBufferBeginInfo;
            }

            VkSubmitInfo submitInfo(const std::vector<VkCommandBuffer>& commandBuffers, const VkPipelineStageFlags waitStages) {
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = commandBuffers.size();
                submitInfo.pCommandBuffers = commandBuffers.data();
                submitInfo.pWaitDstStageMask = &waitStages;
                return submitInfo;
            }

            VkSubmitInfo submitInfo(VkCommandBuffer commandBuffer) {
                VkSubmitInfo submitInfo{};
                submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount = 1;
                submitInfo.pCommandBuffers = &commandBuffer;
                return submitInfo;
            }

            VkPresentInfoKHR presentInfo(const std::vector<VkSwapchainKHR>& swapchains, const std::vector<uint32_t>& imageIndices) {
                VkPresentInfoKHR presentInfo{};
                presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.pSwapchains = swapchains.data();
                presentInfo.swapchainCount = swapchains.size();
                presentInfo.pImageIndices = imageIndices.data();
                return presentInfo;
            }

        }

        namespace Container {

            VkImageCreateInfo imageCreateInfo(const VkFormat& format, const VkImageTiling& tiling, const VkExtent3D& extent, VkImageUsageFlagBits usage, VkImageType imageType) {
                VkImageCreateInfo imageCreateInfo{};
                imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageCreateInfo.imageType = imageType;
                imageCreateInfo.format = format;
                imageCreateInfo.tiling = tiling;
                imageCreateInfo.extent = extent;
                imageCreateInfo.mipLevels = 1;
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageCreateInfo.usage = usage;
                imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
                return imageCreateInfo;
            }

            VkImageViewCreateInfo imageViewCreateInfo(const VkImage& image, const VkFormat& format, const VkImageViewType& viewType, const VkImageViewCreateFlags& flags) {
                VkImageViewCreateInfo imageViewCreateInfo{};
                imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                imageViewCreateInfo.format = format;
                imageViewCreateInfo.components = {
                    VK_COMPONENT_SWIZZLE_R,
                    VK_COMPONENT_SWIZZLE_G,
                    VK_COMPONENT_SWIZZLE_B,
                    VK_COMPONENT_SWIZZLE_A
                };
                imageViewCreateInfo.subresourceRange = {
                    VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1
                };
                imageViewCreateInfo.viewType = viewType;
                imageViewCreateInfo.flags = flags;
                imageViewCreateInfo.image = image;
                return imageViewCreateInfo;
            }

            VkMemoryAllocateInfo memoryAllocateInfo(const VkDeviceSize& size, const uint32_t& memoryTypeIndex) {
                VkMemoryAllocateInfo memoryAllocateInfo{};
                memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                memoryAllocateInfo.allocationSize = size;
                memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
                return memoryAllocateInfo;
            }

        }

    }

}
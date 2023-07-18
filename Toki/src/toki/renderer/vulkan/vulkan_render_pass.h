#pragma once

#include "tkpch.h"

namespace Toki {

    class VulkanRenderPass {
    public:
        struct RenderPassSpec {
            std::vector<VkAttachmentDescription> attachments;
            std::vector<VkSubpassDependency> dependencies;
            std::vector<VkSubpassDescription> subpasses;
        };

        struct AttachmentReferences {
            std::vector<VkAttachmentReference> colorAttachmentReferences;
            VkAttachmentReference depthAttachmentReference;

            void addColorReferences(const std::vector<VkImageLayout>& imageLayouts) {
                for (const auto& imageLayout : imageLayouts) {
                    addColorReference(imageLayout);
                }
            }

            void addColorReference(VkImageLayout imageLayout) {
                colorAttachmentReferences.push_back({ .attachment = static_cast<uint32_t>(colorAttachmentReferences.size()), .layout = imageLayout });
            }

            // Needs to be last added
            void addDepthReference(VkImageLayout imageLayout) {
                depthAttachmentReference = { .attachment = static_cast<uint32_t>(colorAttachmentReferences.size()), .layout = imageLayout };
            }
        };

        void cleanup();

        VkRenderPass getRenderPass() const { return renderPass; }

        void beginRenderPass(VkCommandBuffer cmd, VkRenderPassBeginInfo* renderPassBeginInfo);
        void endRenderPass(VkCommandBuffer cmd);

        static VulkanRenderPass create(const RenderPassSpec& renderPassSpec);

        static VkAttachmentDescription createColorAttachmentDescription(VkFormat format);
        static VkAttachmentDescription createDepthAttachmentDescription(VkFormat format);
        static VkSubpassDescription createSubpassDescription(const AttachmentReferences& attachmentReferences);

        VulkanRenderPass() = default;
    private:

        VkRenderPass renderPass;
    };

}
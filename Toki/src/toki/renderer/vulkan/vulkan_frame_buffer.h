#pragma once

#include "tkpch.h"

namespace Toki {

    class VulkanFrameBuffer {
    public:
        VulkanFrameBuffer() = default;

        VkFramebuffer getFrameBuffer() { return frameBuffer; }

        void cleanup();
        static VulkanFrameBuffer create(VkRenderPass renderPass, VkExtent2D extent, const std::vector<VkImageView>& attachments = {});

    private:
        VkFramebuffer frameBuffer;
    };

}
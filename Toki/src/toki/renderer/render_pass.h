#pragma once

#include "core/core.h"

namespace Toki {

    enum class RenderTarget {
        None, Swapchain, Texture
    };

    enum class Format {
        RGBA8, R32, R32G32i, Depth, DepthStencil
    };

    enum class Samples {
        Sample1, Sample2, Sample4, Sample8, Sample16, Sample32, Sample64
    };

    enum class AttachmentStoreOp {
        DontCare, Store
    };

    enum class AttachmentLoadOp {
        DontCare, Load, Clear
    };

    enum class InitialLayout {
        Undefined, Present
    };

    using SubpassBits = uint8_t;

    struct Attachment {
        Format format;
        Samples samples = Samples::Sample1;
        AttachmentLoadOp loadOp = AttachmentLoadOp::DontCare;
        AttachmentStoreOp storeOp = AttachmentStoreOp::Store;
        RenderTarget target = RenderTarget::Texture;
        bool isDepthAttachment = false;
        InitialLayout initialLayout = InitialLayout::Undefined;
        SubpassBits includeSubpassBits = 1; // Bits representing in which subpass this attachment is included
        SubpassBits inputSubpassBits = 0; // Bits representing in which subpass this attachment is an input attachment
        SubpassBits resolveSubpassBits = 0; // Bits representing in which subpass this attachment is a resolve attachment
        SubpassBits dependantSubpassBits = 0;
    };

    struct RenderPassConfig {
        uint8_t nSubpasses;
        std::vector<Attachment> attachments;
    };

    class RenderPass {
    public:
        static Ref<RenderPass> create(const RenderPassConfig& config);

        RenderPass(const RenderPassConfig& config);
        virtual ~RenderPass() = default;

    protected:
        RenderPassConfig config;
    };

}
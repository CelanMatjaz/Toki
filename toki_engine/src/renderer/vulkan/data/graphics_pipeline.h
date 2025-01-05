#pragma once

#include <vulkan/vulkan.h>

#include "core/core.h"
#include "core/macros.h"
#include "renderer/shader.h"
#include "renderer/vulkan/data/attachment_hash.h"
#include "renderer/vulkan/renderer_state.h"

namespace toki {

struct GraphicsPipeline {
    struct Config {
        std::filesystem::path vertex_shader_path{};
        std::filesystem::path fragment_shader_path{};
        std::vector<Attachment> attachments;
    };

    static GraphicsPipeline create(Ref<RendererContext> ctx, const Config& config);
    static void cleanup(Ref<RendererContext> ctx, GraphicsPipeline& pipeline);

    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
    AttachmentsHash attachmentHash{};
};

}  // namespace toki

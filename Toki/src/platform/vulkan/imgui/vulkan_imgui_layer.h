#pragma once

#include "renderer/imgui/imgui_layer.h"
#include "platform/vulkan/vulkan_framebuffer.h"
#include "platform/vulkan/vulkan_render_pass.h"

namespace Toki {

    class VulkanImGuiLayer : public ImGuiLayer {
    public:
        VulkanImGuiLayer() = default;
        virtual ~VulkanImGuiLayer() = default;

        virtual void onEvent(Event& e) override;
        virtual void onAttach() override;
        virtual void onDetach() override;
        virtual void onUpdate(float deltaTime) {};

        virtual void startImGuiFrame() override;
        virtual void endImGuiFrame() override;

    private:
        VkDescriptorPool descriptorPool;
        Ref<RenderPass> renderPass;
        Ref<Framebuffer> framebuffer;
    };

}
#pragma once

#include "tkpch.h"
#include "toki/core/layer.h"

namespace Toki {

    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer() = default;
        virtual ~ImGuiLayer() = default;

        virtual void onEvent(Event& e) override;
        virtual void onAttach() override;
        virtual void onDetach() override;
        virtual void onUpdate(float deltaTime) override;

        void startImGuiFrame();
        void endImGuiFrame();

    private:
        VkDescriptorPool descriptorPool;
    };

}
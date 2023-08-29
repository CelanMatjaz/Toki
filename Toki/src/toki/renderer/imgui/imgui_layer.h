#pragma once

#include "core/core.h"
#include "core/layer.h"

namespace Toki {

    class ImGuiLayer : public Layer {
    public:
        static Ref<ImGuiLayer> create();

        ImGuiLayer() = default;
        virtual ~ImGuiLayer() = default;

        virtual void startImGuiFrame() = 0;
        virtual void endImGuiFrame() = 0;
    };

}
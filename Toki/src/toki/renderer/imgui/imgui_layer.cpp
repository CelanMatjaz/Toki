#include "tkpch.h"
#include "imgui_layer.h"
#include "platform/vulkan/imgui/vulkan_imgui_layer.h"

namespace Toki {

    Ref<ImGuiLayer> ImGuiLayer::create() {
        return createRef<VulkanImGuiLayer>();
    }

}
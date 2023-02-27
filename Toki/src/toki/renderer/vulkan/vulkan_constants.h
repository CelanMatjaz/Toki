#pragma once

#include "tkpch.h"

namespace Toki {
    static const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    static const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
}
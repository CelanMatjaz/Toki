#pragma once

#include <vulkan/vulkan.h>

#include <format>

#include "core/core.h"

template <>
struct std::formatter<VkResult> : std::formatter<std::string> {
    auto format(const VkResult& v, std::format_context& ctx) const {
        return std::formatter<std::string>::format(std::format("{}", (u32) v), ctx);
    }
};

#pragma once

#include <toki/core.h>
#include <vulkan/vulkan.h>

#include <format>

template <>
struct std::formatter<VkResult> : std::formatter<std::string> {
    auto format(const VkResult& v, std::format_context& ctx) const {
        return std::formatter<std::string>::format(std::format("{}", (u32) v), ctx);
    }
};

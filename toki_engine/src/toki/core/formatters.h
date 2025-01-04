#pragma once

#include <filesystem>
#include <format>

template <>
struct std::formatter<std::filesystem::path> : std::formatter<std::string> {
    auto format(const std::filesystem::path& p, std::format_context& ctx) const {
        return std::formatter<std::string>::format(p.string(), ctx);
    }
};

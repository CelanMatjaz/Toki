#pragma once

#include <filesystem>

#include "core/macros.h"
#include "renderer/renderer_types.h"

namespace toki {

class Shader {
public:
    struct Config {
        std::filesystem::path vertex_shader_path{};
        std::filesystem::path fragment_shader_path{};
        std::vector<Attachment> attachments;
    };

    Shader(const Config& config): m_config(config) {};

protected:
    Config m_config{};
};

}  // namespace toki

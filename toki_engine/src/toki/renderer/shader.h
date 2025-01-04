#pragma once

#include <filesystem>

namespace toki {

class Shader {
public:
    struct Config {
        std::filesystem::path vertex_shader_path{};
        std::filesystem::path fragment_shader_path{};
    };

    static std::shared_ptr<Shader> create(const Config& config);

    Shader(const Config& config);
    Shader() = delete;
    ~Shader() = default;
};

}  // namespace toki

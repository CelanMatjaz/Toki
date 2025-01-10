#pragma once

#include <filesystem>

#include "renderer/renderer_types.h"

namespace toki {

namespace configs {

struct Shader {
    ShaderStage stage;
    std::filesystem::path path;
};

struct ShaderOptions {
    b8 depth_test_enable : 1;
    b8 depth_write_enable : 1;
    CompareOp depth_compare_op;
    PrimitiveTopology primitive_topology;
    CullMode cull_mode;
    PolygonMode polygon_mode;
    FrontFace front_face;
};

struct ShaderConfig {
    std::string name;
    ShaderType type;
    ShaderOptions options;
    std::vector<Shader> stages;
    std::vector<VertexBindingDescription> bindings;
    std::vector<VertexAttributeDescription> attributes;
};

ShaderConfig load_shader_config(std::filesystem::path path);

}  // namespace configs

}  // namespace toki

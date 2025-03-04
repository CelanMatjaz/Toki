#pragma once

#include <yaml-cpp/yaml.h>

#include "resources/configs/shader_config_loader.h"

template <>
struct YAML::convert<toki::configs::ShaderOptions> {
    static bool decode(const YAML::Node& node, toki::configs::ShaderOptions& rhs) {
        using namespace toki;
        rhs = {};

        rhs.depth_test_enable = node["depth_test_enable"].as<b8>();
        rhs.depth_write_enable = node["depth_write_enable"].as<b8>();

        {
            auto val = node["depth_compare_op"].as<std::string>();
            if (val == "less") {
                rhs.depth_compare_op = CompareOp::Less;
            } else if (val == "equal") {
                rhs.depth_compare_op = CompareOp::Equal;
            } else if (val == "less_or_equal") {
                rhs.depth_compare_op = CompareOp::LessOrEqual;
            } else if (val == "greater") {
                rhs.depth_compare_op = CompareOp::Greater;
            } else if (val == "greater_or_equal") {
                rhs.depth_compare_op = CompareOp::GreaterOrEqual;
            } else if (val == "always") {
                rhs.depth_compare_op = CompareOp::Always;
            } else {
                return false;
            }
        }

        {
            auto val = node["primitive_topology"].as<std::string>();
            if (val == "point_list") {
                rhs.primitive_topology = PrimitiveTopology::PointList;
            } else if (val == "line_list") {
                rhs.primitive_topology = PrimitiveTopology::LineList;
            } else if (val == "line_strip") {
                rhs.primitive_topology = PrimitiveTopology::LineStrip;
            } else if (val == "triangle_list") {
                rhs.primitive_topology = PrimitiveTopology::TriangleList;
            } else if (val == "triangle_strip") {
                rhs.primitive_topology = PrimitiveTopology::TriangleStrip;
            } else if (val == "triangle_fan") {
                rhs.primitive_topology = PrimitiveTopology::TriangleFan;
            } else if (val == "line_list_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::LineListWithAdjacency;
            } else if (val == "line_strip_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::LineStripWithAdjacency;
            } else if (val == "triangle_list_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::TriangleListWithAdjacency;
            } else if (val == "triangle_strip_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::TriangleStripWithAdjacency;
            } else if (val == "patch_list") {
                rhs.primitive_topology = PrimitiveTopology::PatchList;
            } else {
                return false;
            }
        }

        {
            auto val = node["cull_mode"].as<std::string>();
            if (val == "none") {
                rhs.cull_mode = CullMode::None;
            } else if (val == "front") {
                rhs.cull_mode = CullMode::Front;
            } else if (val == "back") {
                rhs.cull_mode = CullMode::Back;
            } else if (val == "both" || val == "front_and_back") {
                rhs.cull_mode = CullMode::Both;
            } else {
                return false;
            }
        }

        {
            auto val = node["polygon_mode"].as<std::string>();
            if (val == "fill") {
                rhs.polygon_mode = PolygonMode::Fill;
            } else if (val == "line") {
                rhs.polygon_mode = PolygonMode::Line;
            } else if (val == "point") {
                rhs.polygon_mode = PolygonMode::Point;
            } else {
                return false;
            }
        }

        {
            auto val = node["front_face"].as<std::string>();
            if (val == "clockwise") {
                rhs.front_face = FrontFace::Clockwise;
            } else if (val == "counter_clockwise") {
                rhs.front_face = FrontFace::CounterClockwise;
            } else {
                return false;
            }
        }

        return true;
    }
};

template <>
struct YAML::convert<toki::VertexBindingDescription> {
    static bool decode(const YAML::Node& node, toki::VertexBindingDescription& rhs) {
        using namespace toki;
        rhs = {};

        rhs.binding = node["binding"].as<u32>();
        rhs.stride = node["stride"].as<u32>();

        {
            auto val = node["input_rate"].as<std::string>();

            if (val == "vertex") {
                rhs.inputRate = VertexInputRate::Vertex;
            } else if (val == "instance") {
                rhs.inputRate = VertexInputRate::Instance;
            } else {
                return false;
            }
        }

        return true;
    }
};

template <>
struct YAML::convert<toki::VertexAttributeDescription> {
    static bool decode(const YAML::Node& node, toki::VertexAttributeDescription& rhs) {
        using namespace toki;
        rhs = {};

        rhs.location = node["location"].as<u32>();
        rhs.binding = node["binding"].as<u32>();

        if (auto& n = node["offset"]; n.IsDefined()) {
            rhs.offset = n.as<u32>();
        }

        {
            auto val = node["type"].as<std::string>();
            if (val == "float1") {
                rhs.format = VertexFormat::FLOAT1;
            } else if (val == "float2") {
                rhs.format = VertexFormat::FLOAT2;
            } else if (val == "float3") {
                rhs.format = VertexFormat::FLOAT3;
            } else if (val == "float4") {
                rhs.format = VertexFormat::FLOAT4;
            } else {
                return false;
            }
        }

        return true;
    }
};

template <>
struct YAML::convert<toki::configs::Shader> {
    static bool decode(const YAML::Node& node, toki::configs::Shader& rhs) {
        using namespace toki;
        rhs = {};

        rhs.path = node["path"].as<std::string>();

        {
            auto val = node["type"].as<std::string>();
            if (val == "vertex") {
                rhs.stage = ShaderStage::Vertex;
            } else if (val == "fragment") {
                rhs.stage = ShaderStage::Fragment;
            } else {
                return false;
            }
        }

        return true;
    }
};

template <>
struct YAML::convert<toki::configs::ShaderConfig> {
    static bool decode(const YAML::Node& node, toki::configs::ShaderConfig& rhs) {
        using namespace toki;
        rhs = {};

        rhs.name = node["name"].as<std::string>();

        auto type = node["type"].as<std::string>();
        if (type == "graphics") {
            rhs.type = ShaderType::GRAPHICS;
        } else {
            return false;
        }

        rhs.stages = node["stages"].as<std::vector<toki::configs::Shader>>();
        rhs.options = node["options"].as<toki::configs::ShaderOptions>();
        rhs.bindings = node["bindings"].as<std::vector<VertexBindingDescription>>();
        rhs.attributes = node["attributes"].as<std::vector<VertexAttributeDescription>>();

        return true;
    }
};

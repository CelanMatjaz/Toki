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
                rhs.depth_compare_op = CompareOp::LESS;
            } else if (val == "equal") {
                rhs.depth_compare_op = CompareOp::EQUAL;
            } else if (val == "less_or_equal") {
                rhs.depth_compare_op = CompareOp::LESS_OR_EQUAL;
            } else if (val == "greater") {
                rhs.depth_compare_op = CompareOp::GREATER;
            } else if (val == "greater_or_equal") {
                rhs.depth_compare_op = CompareOp::GREATER_OR_EQUAL;
            } else if (val == "equal") {
                rhs.depth_compare_op = CompareOp::ALWAYS;
            } else {
                return false;
            }
        }

        {
            auto val = node["primitive_topology"].as<std::string>();
            if (val == "point_list") {
                rhs.primitive_topology = PrimitiveTopology::POINT_LIST;
            } else if (val == "line_list") {
                rhs.primitive_topology = PrimitiveTopology::LINE_LIST;
            } else if (val == "line_strip") {
                rhs.primitive_topology = PrimitiveTopology::LINE_STRIP;
            } else if (val == "triangle_list") {
                rhs.primitive_topology = PrimitiveTopology::TRIANGLE_LIST;
            } else if (val == "triangle_strip") {
                rhs.primitive_topology = PrimitiveTopology::TRIANGLE_STRIP;
            } else if (val == "triangle_fan") {
                rhs.primitive_topology = PrimitiveTopology::TRIANGLE_FAN;
            } else if (val == "line_list_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::LINE_LIST_WITH_ADJACENCY;
            } else if (val == "line_strip_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY;
            } else if (val == "triangle_list_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY;
            } else if (val == "triangle_strip_with_adjacency") {
                rhs.primitive_topology = PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY;
            } else if (val == "patch_list") {
                rhs.primitive_topology = PrimitiveTopology::PATCH_LIST;
            } else {
                return false;
            }
        }

        {
            auto val = node["cull_mode"].as<std::string>();
            if (val == "none") {
                rhs.cull_mode = CullMode::NONE;
            } else if (val == "front") {
                rhs.cull_mode = CullMode::FRONT;
            } else if (val == "back") {
                rhs.cull_mode = CullMode::BACK;
            } else if (val == "both" || val == "front_and_back") {
                rhs.cull_mode = CullMode::BOTH;
            } else {
                return false;
            }
        }

        {
            auto val = node["polygon_mode"].as<std::string>();
            if (val == "fill") {
                rhs.polygon_mode = PolygonMode::FILL;
            } else if (val == "line") {
                rhs.polygon_mode = PolygonMode::LINE;
            } else if (val == "point") {
                rhs.polygon_mode = PolygonMode::POINT;
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
                rhs.inputRate = VertexInputRate::VERTEX;
            } else if (val == "instance") {
                rhs.inputRate = VertexInputRate::INSTANCE;
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
                rhs.stage = ShaderStage::VERTEX;
            } else if (val == "fragment") {
                rhs.stage = ShaderStage::FRAGMENT;
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

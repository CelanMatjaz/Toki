#pragma once

#include "math/types.h"
#include "math/vector.h"
#include "renderer/defines.h"

namespace toki {

enum class color_format : u8 {
    NONE,
    R8,
    RGBA8,
    DEPTH,
    STENCIL,
    DEPTH_STENCIL,

    COLOR_FORMAT_COUNT
};

enum class render_target_load_op : u8 {
    LOAD,
    CLEAR,
    DONT_CARE,
};

enum class render_target_store_op : u8 {
    STORE,
    DONT_CARE,
};

struct render_target {
    color_format color_format = color_format::NONE;
    render_target_load_op load_op : 2 = render_target_load_op::LOAD;
    render_target_store_op store_op : 2 = render_target_store_op::STORE;
    bool presentable : 2 = false;
};

enum class shader_stage : u8 {
    VERTEX,
    FRAGMENT
};

enum class vertex_input_rate : u8 {
    VERTEX,
    INSTANCE,
};

enum class vertex_format : u8 {
    FLOAT1,
    FLOAT2,
    FLOAT3,
    FLOAT4,
};

struct VertexAttributeDescription {
    uint32_t location;
    uint32_t binding;
    vertex_format format;
    uint32_t offset;
};

struct VertexBindingDescription {
    uint32_t binding;
    uint32_t stride;
    vertex_input_rate inputRate;
};

enum class buffer_type : u8 {
    NONE,
    VERTEX,
    INDEX
};

enum class buffer_usage : u8 {
    NONE,
    STATIC,
    DYNAMIC,
};

struct begin_pass_config {
    rect2d render_area{};
    handle framebuffer_handle{};
    vector4<f32> clear_value;
};

}  // namespace toki

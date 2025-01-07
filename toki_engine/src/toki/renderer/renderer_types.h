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
    render_target_load_op load_op : 3 = render_target_load_op::LOAD;
    render_target_store_op store_op : 3 = render_target_store_op::DONT_CARE;
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
    i32 present_target_index = -1;
    u32 color_render_target_count = 0;
    color_format render_target_color_formats[MAX_COLOR_RENDER_TARGETS]{};
    color_format render_target_depth_format = color_format::NONE;
    color_format render_target_stencil_format = color_format::NONE;
    vector4<f32> clear_value;
};

}  // namespace toki

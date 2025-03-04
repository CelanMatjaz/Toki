#version 450

layout(location = 0) out vec3 out_color;

layout(location = 0) in vec2 in_model_pos;
layout(location = 1) in vec2 in_instance_pos;
layout(location = 2) in vec3 in_instance_color; 

layout(push_constant) uniform constants {
	mat4 mvp;
    float screen_width;
    float screen_height;
    float cell_size;
} push;

vec3 colors[4] = vec3[](
    vec3(1.0, 1.0, 1.0),
    vec3(1.0, 0.47, 0.361),
    vec3(0.35, 0.87, 0.612),
    vec3(0.43, 0.25, 0.88)
);

void main() {
    vec2 normalized_position = vec2(in_instance_pos.x / push.screen_width, in_instance_pos.y / push.screen_height);

    float bottom_left_weight = (1.0 - normalized_position.x) * (1.0 - normalized_position.y);
    float bottom_right_weight = normalized_position.x * (1.0 - normalized_position.y);
    float top_left_weight = (1.0 - normalized_position.x) * normalized_position.y;
    float top_right_weight = normalized_position.x * normalized_position.y;

    gl_Position = push.mvp * vec4(vec3(in_model_pos + in_instance_pos, 0.0), 1.0);
    // out_color = in_instance_color;
    out_color = in_instance_color * (
        bottom_left_weight * colors[0] +
        bottom_right_weight * colors[1] +
        top_left_weight * colors[2] +
        top_right_weight  * colors[3]
    );
}

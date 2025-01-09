#version 450

layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0, -1),
    vec2(1, 1),
    vec2(-1, 1)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(push_constant) uniform constants {
	mat4 mvp;
} PushConstants;

void main() {
    gl_Position = PushConstants.mvp * vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}

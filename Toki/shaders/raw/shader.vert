#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in uint imageIndex;
layout (location = 4) in vec2 uv;

layout (location = 0) out vec4 outColor;
layout(location = 1) out vec2 fragTexCoord;

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 projection;
    mat4 view;
    mat4 model;
    vec3 lightPosition;
} ubo;

layout (push_constant) uniform constants {
	mat4 render_matrix;
} PushConstants;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(4.0, 0.0, 2.0));
const float AMBIENT = 0.02;

void main() {
    mat4 renderMatrix = ubo.projection * ubo.view * ubo.model;

    gl_Position = renderMatrix * vec4(position, 1.0);

    vec3 normalWorldSpace = normalize(mat3(renderMatrix) * normal);
    float lightIntensity = AMBIENT + max(dot(normalWorldSpace, normalize(normalize(ubo.lightPosition))), 0);

    outColor = lightIntensity * color;
    fragTexCoord = uv;
    // outColor = color;
}
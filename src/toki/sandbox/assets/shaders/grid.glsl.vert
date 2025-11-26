#version 460

layout(location = 0) out vec3 out_position;

layout(set = 0, binding = 0) uniform MyUniform {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 position;
} ubo;

const vec3 vertices[4] = vec3[4](
	vec3(-1.0, 0.0,  1.0),
	vec3( 1.0, 0.0,  1.0),
	vec3(-1.0, 0.0, -1.0),
	vec3( 1.0, 0.0, -1.0)
);

const int indices[6] = int[6](0, 1, 2, 2, 1, 3);

const float GRID_SIZE = 10.0;

void main() {
	int index = indices[gl_VertexIndex];
	vec3 pos3 = vertices[index] * GRID_SIZE;
	pos3.x += ubo.position.x;
	pos3.z += ubo.position.z;

	out_position = pos3;

	vec4 pos = vec4(pos3, 1.0);
	gl_Position = ubo.projection * ubo.view * pos;
}

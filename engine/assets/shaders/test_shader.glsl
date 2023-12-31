$type frag
#version 460

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 inColor;

void main() {
   outColor = vec4(inColor, 1.0); 
}


$type vert
#version 460

// layout(location = 0) in vec3 position;
 
layout(location = 0) out vec3 outColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    // gl_Position = vec4(position, 1.0);
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    outColor = colors[gl_VertexIndex];
}

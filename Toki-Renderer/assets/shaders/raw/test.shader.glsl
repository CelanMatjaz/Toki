#type frag
#version 450

layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform color {
    vec4 color;
} addedColor;

void main() {
	outFragColor = vec4(1, 1, 1, 1) * addedColor.color;	
}

// ---------------------------

#type vert
#version 450

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 instancePosition;

layout(push_constant) uniform constants {
	vec2 offsetPosition;
} PushConstants;

void main() {
    gl_Position = vec4(position.x + instancePosition.x + PushConstants.offsetPosition.x, position.y + instancePosition.y + PushConstants.offsetPosition.y, 0.0, 1.0);
}


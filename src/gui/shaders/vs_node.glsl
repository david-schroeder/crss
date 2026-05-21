#version 330 core

layout (location = 0) in vec2 aPos_screen;
layout (location = 1) in uvec3 aBase;
layout (location = 2) in uint aType;

out vec2 offset;
flat out uint type;

uniform mat4 uView;
uniform mat4 uProjection;

void main() {
    vec4 node_position = vec4(aBase, 1.0f);
    vec4 node_pos_view = uView * node_position;
    vec4 offsetted_pos = node_pos_view + vec4(aPos_screen, 0.0f, 0.0f);
    gl_Position = uProjection * offsetted_pos;
    offset = aPos_screen;
    type = aType;
}

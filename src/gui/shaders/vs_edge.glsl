#version 330 core

layout (location = 0) in vec2 aPos_screen;
layout (location = 1) in uvec3 aFrom;
layout (location = 2) in uvec3 aTo;

out vec2 uv;
out float dist;

const float EDGE_QUAD_HALFWIDTH = 0.07;

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 view_from = view * vec4(aFrom, 1.0f);
    vec4 view_to = view * vec4(aTo, 1.0f);
    vec4 d = view_to - view_from;
    vec3 p = normalize(cross(d.xyz, vec3(0.0, 0.0, 1.0))) * EDGE_QUAD_HALFWIDTH;
    vec4 p4 = vec4(p, 0.0);

    vec4 viewpos_final = view_from + aPos_screen.x * d + aPos_screen.y * p4;

    gl_Position = projection * viewpos_final;
    uv = vec2(aPos_screen.x, (aPos_screen.y + 1.0) / 2.0);
    dist = length(d);
}

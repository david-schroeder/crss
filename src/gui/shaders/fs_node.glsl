#version 330 core

in vec2 offset;
flat in uint type;

out vec4 FragColor;

const float RADIUS = 0.25;
const float RADIUS_SQUARED = RADIUS * RADIUS;

void main() {
    if (dot(offset, offset) > RADIUS_SQUARED) discard;
    // Circular node with antialiased edge
    float alpha = smoothstep(RADIUS_SQUARED, RADIUS_SQUARED-0.01, dot(offset, offset));

    FragColor = vec4(0.8f, 0.4f, 0.6f, alpha);
}

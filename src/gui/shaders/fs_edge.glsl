#version 330 core

in vec2 uv;
in float dist;

out vec4 FragColor;

const float PADDING_TIP = 0.26;
const float PADDING_TAIL = 0.3;
const float HEAD_LENGTH = 0.1;
const float HALF_WIDTH = 0.1;

const vec4 COLOR = vec4(0.8, 0.8, 0.8, 1.0);

// TODO: make constants uniforms

void main() {
    float padding_tip_uv = 1 - PADDING_TIP / dist;
    float head_length_uv = HEAD_LENGTH / dist;
    float padding_tail_uv = PADDING_TAIL / dist;

    if (uv.x > padding_tip_uv) {
        discard;
    }
    if (uv.x < padding_tail_uv) {
        discard;
    }
    if (uv.x < padding_tip_uv - head_length_uv) {
        if (abs(uv.y - 0.5) > HALF_WIDTH) discard;
        FragColor = COLOR;
        return;
    }

    float tip_dist = padding_tip_uv - uv.x;
    float cutoff_y = tip_dist / (2.0 * HEAD_LENGTH);
    if (abs(uv.y - 0.5) > cutoff_y) discard;

    FragColor = COLOR;
}

#version 330 core

in vec2 worldPos;

out vec4 FragColor;

uniform vec3 uCameraPos;

const float FADE_DIST = 50.0;

const float lineWidth = 0.05;

void main() {

    vec4 uvDDXY = vec4(dFdx(worldPos), dFdy(worldPos));
    vec2 uvDeriv = vec2(length(uvDDXY.xz), length(uvDDXY.yw));
    vec2 drawWidth = clamp(vec2(lineWidth), uvDeriv, vec2(0.5));
    vec2 lineAA = uvDeriv * 1.5;
    vec2 gridUV = 1.0 - abs(fract(worldPos) * 2.0 - 1.0);
    vec2 grid2 = smoothstep(drawWidth + lineAA, drawWidth - lineAA, gridUV);
    grid2 *= clamp(vec2(lineWidth) / drawWidth, 0.0, 1.0);
    grid2 = mix(grid2, vec2(lineWidth), clamp(uvDeriv * 2.0 - 1.0, 0.0, 1.0));
    float grid = mix(grid2.x, 1.0, grid2.y);

    vec3 color = vec3(0.5);

    if (abs(worldPos.x) < 0.03) color = vec3(0.0, 0.0, 0.8);
    if (abs(worldPos.y) < 0.03) color = vec3(0.8, 0.0, 0.0);
    
    if (grid < 0.05) discard;

    float camDist = length(worldPos - uCameraPos.xz);
    float alpha = clamp(exp(-camDist/8) * FADE_DIST, 0.0, 1.0);
    // genuinely using alpha introduces some weird bugs in the |x| < z region

    FragColor = vec4(grid * color * alpha, 1.0);
}

#version 450

// Declaring output variable.

// [layout] qualifier takes location value.
// based on setup of graphic pipelinet, the fragment shader can
// output to multiple different locations. for now using location 0 only.

// [out] qualifier specifies that the variable is going tobe used as an output of this fn.
// with type "vec4" and name "outColor".
layout (location = 0) out vec4 outColor;
layout(push_constant) uniform Push {
    vec2 offset;
    vec3 color;
} push;
void main() {
    // Fragment runs on per fragment basis NOT full image
    // => colors fragment(pixel or subpixels) one by one.
    // Inputs are fragments from rasterization stage:
    // vertices position -> [Rasterization] -> pixels/fragments inside geometry.
    // R,G,B,Alpha(opaqueness), [0.0-1.0] value range.
    // Can initialize vec4 with vec4(vec3, float).
    outColor = vec4(push.color, 1.0);
}
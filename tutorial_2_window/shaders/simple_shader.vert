// Setting GLSL version to 450.
#version 450

// position forms a triangle.
// x = width to the right.
// y = height to the bottom.
vec2 positions[3] = vec2[] (
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

// Going to get executed for each vertex we have.
// INPUT: get vertex from inpupt assembler stage.
// OUTPUT: need to return position. set by modifiying gl_Position rather than return.
void main() {
    // gl_VertexIndex is the index of current vertex, which is different for every fn invoke.
    // gl_Position is 4d vector that map to output buffer frame img.
    // Z-axis = layer level, ranges from 0(most front) to 1(most back).
    // norm = normalization/divide the rest of the values by the normalization value.
    gl_Position = vec4(positions[gl_VertexIndex], /*Z-axis*/ 0.0, /*norm*/ 1.0);
}
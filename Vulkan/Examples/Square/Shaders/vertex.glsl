#version 450
// Generate a 2-triangle strip square centered in NDC, no vertex buffer needed.
vec2 positions[4] = vec2[](
    vec2(-0.5, -0.5),
    vec2( 0.5, -0.5),
    vec2(-0.5,  0.5),
    vec2( 0.5,  0.5)
);
void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}

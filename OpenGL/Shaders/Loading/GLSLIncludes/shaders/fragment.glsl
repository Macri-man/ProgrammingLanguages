#version 460 core
#include "common.glsl"

out vec4 FragColor;
uniform float time;

void main() {
    FragColor = vec4(rainbow(time), 1.0);
}

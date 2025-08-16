#version 450
layout(binding = 0) uniform UBO {
    vec4 color;
} ubo;
layout(location = 0) out vec4 outColor;
void main() {
    outColor = ubo.color;
}

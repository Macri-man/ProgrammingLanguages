#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D scene;

void main() {
    vec3 color = texture(scene, TexCoords).rgb;
    FragColor = vec4(color, 1.0);
}

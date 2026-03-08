#version 410 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main() {
    FragColor = vec4(vec3(texture(texture_diffuse1, TexCoords)), 1.0);
}

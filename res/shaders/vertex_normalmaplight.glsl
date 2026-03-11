#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec2 aTangent;
layout (location = 4) in vec2 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool reverse_normals;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.TexCoord = aTexCoord;

    gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);

    vec3 T = normalize(vec3(model * vec4(aTangent, 1.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 1.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 1.0)));
    mat3 TBN = transpose(mat3(T, B, N));

    vs_out.TangentLightPos = TBN * lightPos;
    vs_out.TangentViewPos = TBN * viewPos;
    vs_out.TangentFragPos = TBN * vs_out.FragPos;
}

#version 410 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoord;
    vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float height_scale;

vec2 Parallax(vec2 texCoord, vec3 viewDir) {
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;
    vec2 p = viewDir.xy *height_scale;
    vec2 deltaTexCoord = p / numLayers;

    vec2 currentTexCoord = texCoord;
    float currentHeightMapValue = texture(texture_height1, currentTexCoord).r;

    while (currentLayerDepth < currentHeightMapValue) {
        currentTexCoord -= deltaTexCoord;
        currentHeightMapValue = texture(texture_height1, currentTexCoord).r;
        currentLayerDepth += layerDepth;
    }
    return currentTexCoord;
}

void main() {
    vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
    vec2 texCoord = Parallax(fs_in.TexCoord, viewDir);
    if (texCoord.x < 0.0 || texCoord.x > 1.0 || texCoord.y < 0.0 || texCoord.y > 1.0) discard;

    // obtain normal from normal map in range [0,1]
    vec3 normal = texture(texture_normal1, texCoord).rgb;
    // transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0); // this normal is in tangent space

    // get diffuse color
    vec3 color = texture(texture_diffuse1, texCoord).rgb;
    // ambient
    vec3 ambient = 0.1 * color;
    // diffuse
    vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * color;
    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    vec3 specular = vec3(0.2) * spec;
    FragColor = vec4(ambient + diffuse + specular, 1.0);
}

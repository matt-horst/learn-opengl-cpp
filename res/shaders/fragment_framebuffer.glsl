#version 410

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

void main() {
    // FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0); // normal
    // FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0); // inversion

    // gray scale
    // vec4 color = texture(screenTexture, TexCoords);
    // float average = color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722;
    // FragColor = vec4(average, average, average, 1.0);

    vec2 offsets[9] = vec2[](
            vec2(-offset, offset), // top-left
            vec2(0.0f, offset), // top-center
            vec2(offset, offset), // top-right

            vec2(-offset, 0.0f), // center-left
            vec2(0.0f, 0.0f), // center-center
            vec2(offset, 0.0f), // center-right

            vec2(-offset, -offset), // bottom-left
            vec2(0.0f, -offset), // bottom-center
            vec2(offset, -offset) // bottom-right
        );

    // sharpen kernel
    // float kernel[9] = float[](
    //         -1, -1, -1,
    //         -1, 9, -1,
    //         -1, -1, -1
    //     );
    // blur kernel
    // float kernel[9] = float[](
    //         1.0 / 16.0, 1.0 / 16.0, 1.0 / 16.0,
    //         1.0 / 16.0, 4.0 / 16.0, 1.0 / 16.0,
    //         1.0 / 16.0, 1.0 / 16.0, 1.0 / 16.0
    //     );
    // edge detection
    float kernel[9] = float[](
            1, 1, 1,
            1, -8, 1,
            1, 1, 1
        );

    vec3 sampleTex[9];
    for (int i = 0; i < 9; i++) {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }

    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++) {
        col += sampleTex[i] * kernel[i];
    }

    FragColor = vec4(col, 1.0);
}

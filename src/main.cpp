#include <glad/glad.h>
// glad must be included first

#include <GLFW/glfw3.h>

#include <cstdint>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <cstdio>
#include <cstdlib>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.hpp"
#include "model.hpp"
#include "shader.hpp"

void framebuffer_size_callback(GLFWwindow*, int, int);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int load_texture(const std::string& path);
std::uint32_t load_cubemap(std::vector<std::string> faces);
void render_scene(const Shader& shader, std::uint32_t planeVAO, std::uint32_t cubeVAO);
void renderQuad();

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

int main(void) {
    if (!glfwInit()) {
        const char* msg = nullptr;
        glfwGetError(&msg);
        std::fprintf(stderr, "couldn't initialize GLFW: %s", msg);
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        const char* msg = nullptr;
        glfwGetError(&msg);
        std::fprintf(stderr, "couldn't initialize GLFW window: %s", msg);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "couldn't initialize GLAD");
        return EXIT_FAILURE;
    }
    {
        ShaderBuilder sb;
        try {
            sb.m_vertex_src = readFileToString("res/shaders/vertex_parallax.glsl");
            sb.m_fragment_src = readFileToString("res/shaders/fragment_parallax.glsl");
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        std::unique_ptr<Shader> shader;
        try {
            shader = sb.build();
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        float last_frame = 0.0f;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        glfwSetWindowSize(window, 800, 600);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        glEnable(GL_MULTISAMPLE);

        stbi_set_flip_vertically_on_load(true);

        Texture diffuse {"res/textures/bricks2.jpg", ""};
        Texture normalMap {"res/textures/bricks2_normal.jpg", ""};
        Texture heightMap {"res/textures/bricks2_disp.jpg", ""};

        // Model model {"res/models/backpack/backpack.obj"};

        shader->use();
        shader->set_i("texture_diffuse1", 0);
        shader->set_i("texture_normal1", 1);
        shader->set_i("texture_height1", 2);

        glm::vec3 lightPos(0.5f, 1.0f, 0.3f);

        while (!glfwWindowShouldClose(window)) {
            const float current_frame = glfwGetTime();
            const float delta_time = current_frame - last_frame;
            last_frame = current_frame;

            int width, height;
            glfwGetWindowSize(window, &width, &height);

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                camera.pan(PanMovement::FORWARD, delta_time);
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                camera.pan(PanMovement::BACK, delta_time);
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                camera.pan(PanMovement::RIGHT, delta_time);
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                camera.pan(PanMovement::LEFT, delta_time);
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                camera.m_panning_speed = DEFAULT_PANNING_SPEED * 2.0;
            } else {
                camera.m_panning_speed = DEFAULT_PANNING_SPEED;
            }

            // render
            // ------
            glEnable(GL_DEPTH_TEST);  // enable depth testing (is disabled for rendering screen-space quad)

            // make sure we clear the framebuffer's content
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

            shader->use();
            shader->set_mat4("projection", glm::perspective(glm::radians(camera.m_zoom),
                                                            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f));
            shader->set_mat4("view", camera.get_view_matrix());
            shader->set_mat4("model", glm::rotate(glm::mat4(1.0f), glm::radians((float)glfwGetTime() * -10.0f),
                                                  glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f))));
            shader->set_vec3("lightPos", lightPos);
            shader->set_vec3("viewPos", camera.m_position);
            shader->set_f("height_scale", 0.1f);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, diffuse.id);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, normalMap.id);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, heightMap.id);

            renderQuad();


            shader->set_mat4("model", glm::scale(glm::translate(glm::mat4(1.0f), lightPos), glm::vec3(0.1f)));
            renderQuad();

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad() {
    if (quadVAO == 0) {
        // positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        float quadVertices[] = {// positions            // normal         // texcoords  // tangent // bitangent
                                pos1.x, pos1.y,     pos1.z,     nm.x,       nm.y,         nm.z,         uv1.x,
                                uv1.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                                pos2.x, pos2.y,     pos2.z,     nm.x,       nm.y,         nm.z,         uv2.x,
                                uv2.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                                pos3.x, pos3.y,     pos3.z,     nm.x,       nm.y,         nm.z,         uv3.x,
                                uv3.y,  tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                                pos1.x, pos1.y,     pos1.z,     nm.x,       nm.y,         nm.z,         uv1.x,
                                uv1.y,  tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                                pos3.x, pos3.y,     pos3.z,     nm.x,       nm.y,         nm.z,         uv3.x,
                                uv3.y,  tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                                pos4.x, pos4.y,     pos4.z,     nm.x,       nm.y,         nm.z,         uv4.x,
                                uv4.y,  tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z};
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void render_scene(const Shader& shader, std::uint32_t planeVAO, std::uint32_t cubeVAO) {
    shader.use();

    // outer box
    glDisable(GL_CULL_FACE);
    glBindVertexArray(cubeVAO);
    shader.set_mat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(5.0f)));
    shader.set_i("reverse_normals", true);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glEnable(GL_CULL_FACE);

    // render cubes
    glBindVertexArray(cubeVAO);
    shader.set_i("reverse_normals", false);
    shader.set_mat4("model",
                    glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, -3.5f, 0.0f)), glm::vec3(0.5f)));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.set_mat4("model",
                    glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 1.0f)), glm::vec3(0.75f)));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.set_mat4("model",
                    glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-3.0f, -1.0f, 0.0f)), glm::vec3(0.5f)));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.set_mat4("model",
                    glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 1.0f, 1.5f)), glm::vec3(0.5f)));
    glDrawArrays(GL_TRIANGLES, 0, 36);

    shader.set_mat4("model", glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, 2.0f, -3.0f)),
                                                    glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f))),
                                        glm::vec3(0.75f)));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool first_mouse = true;
    if (first_mouse) {
        lastX = xpos;
        lastY = ypos;
        first_mouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.rotate(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.zoom(yoffset); }

std::uint32_t load_cubemap(std::vector<std::string> faces) {
    std::uint32_t id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    std::int32_t width, height, num_channels;
    for (std::uint32_t i = 0; i < faces.size(); i++) {
        std::uint8_t* data = stbi_load(faces[i].c_str(), &width, &height, &num_channels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                         data);
        } else {
            std::printf("Cubemap textured faile to load path: %s\n", faces[i].c_str());
        }

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return id;
}

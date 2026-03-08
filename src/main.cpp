#include <glad/glad.h>
// glad must be included first

#include <GLFW/glfw3.h>

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

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
        ShaderBuilder sb_red, sb_blue, sb_green, sb_yellow;
        try {
            sb_red._m_vertex_src = readFileToString("res/shaders/vertex_uniformbuffer.glsl");
            sb_red._m_fragment_src = readFileToString("res/shaders/fragment_red.glsl");
            sb_blue._m_vertex_src = readFileToString("res/shaders/vertex_uniformbuffer.glsl");
            sb_blue._m_fragment_src = readFileToString("res/shaders/fragment_blue.glsl");
            sb_green._m_vertex_src = readFileToString("res/shaders/vertex_uniformbuffer.glsl");
            sb_green._m_fragment_src = readFileToString("res/shaders/fragment_green.glsl");
            sb_yellow._m_vertex_src = readFileToString("res/shaders/vertex_uniformbuffer.glsl");
            sb_yellow._m_fragment_src = readFileToString("res/shaders/fragment_yellow.glsl");
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        std::unique_ptr<Shader> shader_red, shader_blue, shader_green, shader_yellow;
        try {
            shader_red = sb_red.build();
            shader_blue = sb_blue.build();
            shader_green = sb_green.build();
            shader_yellow = sb_yellow.build();
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        glEnable(GL_DEPTH_TEST);

        float last_frame = 0.0f;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        stbi_set_flip_vertically_on_load(false);

        // Model model_backpack("res/models/backpack/backpack.obj");

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,
                            0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
                            -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f,

                            -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,
                            0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
                            -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,

                            -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, -1.0f, 0.0f,  0.0f,
                            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
                            -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,

                            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f,  0.0f,  0.0f,
                            0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
                            0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

                            -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,
                            0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
                            -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,

                            -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,
                            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
                            -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f};
        float skyboxVertices[] = {// positions
                                  -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                                  1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                                  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                                  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                                  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                                  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                                  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                                  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                                  -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                                  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                                  -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                                  1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
        // cube VAO
        unsigned int cubeVAO, cubeVBO;
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
        // skybox VAO
        std::uint32_t skyboxVAO, skyboxVBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        std::uint32_t uniform_block_index_red = glGetUniformBlockIndex(shader_red->_m_id, "Matrices");
        std::uint32_t uniform_block_index_blue = glGetUniformBlockIndex(shader_blue->_m_id, "Matrices");
        std::uint32_t uniform_block_index_green = glGetUniformBlockIndex(shader_green->_m_id, "Matrices");
        std::uint32_t uniform_block_index_yellow = glGetUniformBlockIndex(shader_yellow->_m_id, "Matrices");

        glUniformBlockBinding(shader_red->_m_id, uniform_block_index_red, 0);
        glUniformBlockBinding(shader_blue->_m_id, uniform_block_index_blue, 0);
        glUniformBlockBinding(shader_green->_m_id, uniform_block_index_green, 0);
        glUniformBlockBinding(shader_yellow->_m_id, uniform_block_index_yellow, 0);

        std::uint32_t uboMatrices;
        glGenBuffers(1, &uboMatrices);

        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

        // load textures
        // -------------
        Texture cubeTexture{"res/textures/container.jpg", ""};
        // Texture floorTexture{"res/textures/metal.png", ""};

        std::vector<std::string> faces = {
            "res/textures/skybox/right.jpg",  "res/textures/skybox/left.jpg",  "res/textures/skybox/top.jpg",
            "res/textures/skybox/bottom.jpg", "res/textures/skybox/front.jpg", "res/textures/skybox/back.jpg",
        };
        const auto skyboxCubemap = load_cubemap(faces);

        glfwSetWindowSize(window, 800, 600);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        glEnable(GL_PROGRAM_POINT_SIZE);

        glm::mat4 projection =
            glm::perspective(glm::radians(camera.m_zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 view = camera.get_view_matrix();
            glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
            glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);

            // cubes
            // model_backpack.draw(*shader);
            // RED
            shader_red->use();
            shader_red->set_mat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-0.75f, 0.75f, 0.0f)));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            shader_blue->use();
            shader_blue->set_mat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.0f)));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            shader_green->use();
            shader_green->set_mat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-0.75f, -0.75f, 0.0f)));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            shader_yellow->use();
            shader_yellow->set_mat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.75f, -0.75f, 0.0f)));
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

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

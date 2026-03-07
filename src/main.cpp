#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <cstdlib>

#include "camera.hpp"
#include "model.hpp"
#include "shader.hpp"

void framebuffer_size_callback(GLFWwindow *, int, int);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
unsigned int load_texture(const std::string &path);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

int main(void) {
    if (!glfwInit()) {
        const char *msg = nullptr;
        glfwGetError(&msg);
        std::fprintf(stderr, "couldn't initialize GLFW: %s", msg);
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        const char *msg = nullptr;
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

        ShaderBuilder sb, sb_blending;
        try {
            sb._m_vertex_src = readFileToString("res/shaders/vertex_depth.glsl");
            sb._m_fragment_src = readFileToString("res/shaders/fragment_depth.glsl");
            sb_blending._m_vertex_src = readFileToString("res/shaders/vertex_depth.glsl");
            sb_blending._m_fragment_src = readFileToString("res/shaders/fragment_blending.glsl");

        } catch (const std::runtime_error &e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        std::unique_ptr<Shader> shader, shader_blending;
        try {
            shader = sb.build();
            shader_blending = sb_blending.build();
        } catch (const std::runtime_error &e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        glEnable(GL_DEPTH_TEST);

        float last_frame = 0.0f;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        stbi_set_flip_vertically_on_load(true);

        // Model model("res/models/backpack/backpack.obj");

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float cubeVertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
            // Front face
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // top-left
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
            // Left face
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
            -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-left
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
                                             // Right face
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-left
            // Bottom face
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  // top-left
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
            // Top face
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-right
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f   // bottom-left
        };
        float planeVertices[] = {
            // positions          // texture Coords (note we set these higher
            // than 1 (together with GL_REPEAT as texture wrapping mode). this
            // will cause the floor texture to repeat)
            5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, 5.0f,  0.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

            5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f, 5.0f,  -0.5f, -5.0f, 2.0f, 2.0f};
        // cube VAO
        unsigned int cubeVAO, cubeVBO;
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glBindVertexArray(0);
        // plane VAO
        unsigned int planeVAO, planeVBO;
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glBindVertexArray(0);

        // vegetation
        float vegetationVertices[] = {// positions          // texture Coords (note we set these higher
                                      // than 1 (together with GL_REPEAT as texture wrapping mode). this
                                      // will cause the floor texture to repeat)
                                      0.0f, -0.5f, 0.0f, 0.0f, 0.0f,

                                      1.0f, -0.5f, 0.0f, 1.0f, 0.0f,

                                      0.0f, 0.5f,  0.0f, 0.0f, 1.0f,

                                      0.0f, 0.5f,  0.0f, 0.0f, 1.0f,

                                      1.0f, -0.5f, 0.0f, 1.0f, 0.0f,

                                      1.0f, 0.5f,  0.0f, 1.0f, 1.0f};

        glm::vec3 vegetation[] = {
            glm::vec3(-1.5f, 0.0f, -0.48f), glm::vec3(1.5f, 0.0f, 0.51f), glm::vec3(0.0f, 0.0f, 0.7f),
            glm::vec3(-0.3f, 0.0f, -2.3f),  glm::vec3(0.5f, 0.0f, -0.6f),
        };

        Texture grassTexture{"res/textures/blending_transparent_window.png", ""};

        unsigned int vegetationVAO, vegetationVBO;
        glGenVertexArrays(1, &vegetationVAO);
        glGenBuffers(1, &vegetationVBO);
        glBindVertexArray(vegetationVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vegetationVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vegetationVertices), &vegetationVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glBindVertexArray(0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // load textures
        // -------------
        Texture cubeTexture{"res/textures/marble.jpg", ""};
        Texture floorTexture{"res/textures/metal.png", ""};

        // shader configuration
        // --------------------
        shader->use();
        shader->set_i("texture1", 0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
        glDepthFunc(GL_LESS);

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

            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glEnable(GL_DEPTH_TEST);

            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // view/projection transformations
            glm::mat4 projection =
                glm::perspective(glm::radians(camera.m_zoom), (float)width / (float)height, 0.1f, 100.0f);
            glm::mat4 view = camera.get_view_matrix();
            shader->use();
            shader->set_mat4("projection", projection);
            shader->set_mat4("view", view);
            shader_blending->use();
            shader_blending->set_mat4("projection", projection);
            shader_blending->set_mat4("view", view);

            {
                // Draw the floor
                shader->use();
                glStencilMask(0x00);
                glBindVertexArray(planeVAO);
                glBindTexture(GL_TEXTURE_2D, floorTexture.id);
                shader->set_mat4("model", glm::mat4(1.0f));
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
            }

            {
                // Draw containers
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilMask(0xFF);

                glm::mat4 m = glm::mat4(1.0f);
                m = glm::translate(m, glm::vec3(-1.0f, 0.0f, -1.0f));
                m = glm::scale(m, glm::vec3(1.0f, 1.0f, 1.0f));
                shader->set_mat4("model", m);

                glBindVertexArray(cubeVAO);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, cubeTexture.id);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                m = glm::mat4(1.0f);
                m = glm::translate(m, glm::vec3(2.0f, 0.0f, 0.0f));
                shader->set_mat4("model", m);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                // glStencilMask(0x00);
                // glDisable(GL_DEPTH_TEST);

                // // Draw Scaled Up Containers
                // shader_single_color->use();
                // m = glm::mat4(1.0f);
                // m = glm::translate( m, glm::vec3(-1.0f, 0.0f, -1.0f));
                // m = glm::scale(m, glm::vec3(1.1f, 1.1f, 1.1f));
                // shader_single_color->set_mat4("model", m);
                //
                // glBindVertexArray(cubeVAO);
                // glActiveTexture(GL_TEXTURE0);
                // glBindTexture(GL_TEXTURE_2D, cubeTexture.id);
                // glDrawArrays(GL_TRIANGLES, 0, 36);
                //
                // m = glm::mat4(1.0f);
                // m = glm::translate(m, glm::vec3(2.0f, 0.0f, 0.0f));
                // m = glm::scale(m, glm::vec3(1.1f, 1.1f, 1.1f));
                // shader_single_color->set_mat4("model", m);
                // glDrawArrays(GL_TRIANGLES, 0, 36);
                //
                // glStencilMask(0xFF);
                // glStencilFunc(GL_ALWAYS, 1, 0xFF);
                // glEnable(GL_DEPTH_TEST);
            }

            {
                // Draw grass
                // sort windows by distance to the camera

                std::map<float, glm::vec3> sorted;
                for (unsigned int i = 0; i < 5; i++) {
                    float distance = glm::length(camera.m_position - vegetation[i]);
                    sorted[distance] = vegetation[i];
                }

                shader_blending->use();

                glBindVertexArray(vegetationVAO);
                glBindTexture(GL_TEXTURE_2D, grassTexture.id);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); it++) {
                    glm::mat4 m{1.0f};
                    m = glm::translate(m, it->second);
                    shader_blending->set_mat4("model", m);
                    glDrawArrays(GL_TRIANGLES, 0, 6);
                }
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); }

float lastX = 400.0f, lastY = 300.0f;
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) { camera.zoom(yoffset); }

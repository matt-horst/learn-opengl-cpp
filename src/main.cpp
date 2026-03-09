#include <glad/glad.h>
// glad must be included first

#include <GLFW/glfw3.h>

#include <cstdint>
#include <iostream>
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

Camera camera(glm::vec3(0.0f, 50.0f, 155.0f));

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
        ShaderBuilder sb, sb_rock;
        try {
            sb.m_vertex_src = readFileToString("res/shaders/vertex_planets.glsl");
            sb.m_fragment_src = readFileToString("res/shaders/fragment_planets.glsl");

            sb_rock.m_vertex_src = readFileToString("res/shaders/vertex_rock.glsl");
            sb_rock.m_fragment_src = readFileToString("res/shaders/fragment_rock.glsl");
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        std::unique_ptr<Shader> shader, shader_rock;
        try {
            shader = sb.build();
            shader_rock = sb_rock.build();
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        glEnable(GL_DEPTH_TEST);

        float last_frame = 0.0f;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);

        // stbi_set_flip_vertically_on_load(false);

        std::uint32_t amount = 100000;
        std::vector<glm::mat4> modelMatrices(amount, glm::mat4(1.0f));
        srand(glfwGetTime());
        float radius = 150.0f;
        float offset = 25.0f;

        for (std::uint32_t i = 0; i < amount; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            float angle = (float) i / (float) amount * 360.0f;
            float displacement = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
            float x = sin(angle) * radius + displacement;
            displacement = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
            float y = displacement * 0.4f;
            displacement = (rand() % (int) (2 * offset * 100)) / 100.0f - offset;
            float z = cos(angle) * radius + offset;
            model = glm::translate(model, glm::vec3(x, y, z));

            float scale = (rand() % 20) / 100.0f + 0.05f;
            model = glm::scale(model, glm::vec3(scale));

            float rotAngle = (rand() % 360);
            model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

            modelMatrices[i] = model;
            // glm::mat4 model = glm::mat4(1.0f);
            // // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            // float angle = (float)i / (float)amount * 360.0f;
            // float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            // float x = sin(angle) * radius + displacement;
            // displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            // float y = displacement * 0.4f;  // keep height of asteroid field smaller compared to width of x and z
            // displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            // float z = cos(angle) * radius + displacement;
            // model = glm::translate(model, glm::vec3(x, y, z));
            //
            // // 2. scale: Scale between 0.05 and 0.25f
            // float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
            // model = glm::scale(model, glm::vec3(scale));
            //
            // // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            // float rotAngle = static_cast<float>((rand() % 360));
            // model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));
            //
            // // 4. now add to list of matrices
            // modelMatrices[i] = model;
        }

        Model planetModel = Model("res/models/planet/planet.obj");
        Model rockModel = Model("res/models/rock/rock.obj");

        std::uint32_t vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

        for (std::uint32_t i = 0; i < rockModel.meshes.size(); i++) {
            // glBindVertexArray(rockModel.meshes[i].vao);
            // for (std::uint32_t j = 0; j < 4; j++) {
            //     glEnableVertexAttribArray(3 + j);
            //     glVertexAttribPointer(3 + j, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *) (j *
            //     sizeof(glm::vec4))); glVertexAttribDivisor(3 + j, 1);
            // }
            // glBindVertexArray(0);
            unsigned int VAO = rockModel.meshes[i].vao;
            glBindVertexArray(VAO);
            // set attribute pointers for matrix (4 times vec4)
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);

            glBindVertexArray(0);
        }

        glfwSetWindowSize(window, 800, 600);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

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

            shader->use();
            shader->set_mat4("model", glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)),
                                                 glm::vec3(4.0f, 4.0f, 4.0f)));
            shader->set_mat4("view", camera.get_view_matrix());
            shader->set_mat4("projection", glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                            0.1f, 1000.0f));
            planetModel.draw(*shader);

            shader_rock->use();
            shader_rock->set_mat4("view", camera.get_view_matrix());
            shader_rock->set_mat4("projection", glm::perspective(glm::radians(45.0f),
                                                                 (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, planetModel.loaded_textures[0].id);
            shader_rock->set_i("texture_diffuse1", 0);
            for (std::uint32_t i = 0; i < rockModel.meshes.size(); i++) {
                glBindVertexArray(rockModel.meshes[i].vao);
                glDrawElementsInstanced(GL_TRIANGLES, static_cast<std::uint32_t>(rockModel.meshes[i].indices.size()),
                                        GL_UNSIGNED_INT, 0, amount);
                glBindVertexArray(0);
            }

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

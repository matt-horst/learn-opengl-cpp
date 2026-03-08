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
        ShaderBuilder sb, sb_skybox;
        try {
            sb._m_vertex_src = readFileToString("res/shaders/vertex_depth.glsl");
            sb._m_fragment_src = readFileToString("res/shaders/fragment_depth.glsl");
            sb_skybox._m_vertex_src = readFileToString("res/shaders/vertex_skybox.glsl");
            sb_skybox._m_fragment_src = readFileToString("res/shaders/fragment_skybox.glsl");
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            return EXIT_FAILURE;
        }

        std::unique_ptr<Shader> shader, shader_skybox;
        try {
            shader = sb.build();
            shader_skybox = sb_skybox.build();
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

        // Model model("res/models/backpack/backpack.obj");

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float cubeVertices[] = {
            // Back face
            -0.5f, -0.5f, -0.5f, 0.0f,
            0.0f,  // Bottom-left
            0.5f, 0.5f, -0.5f, 1.0f,
            1.0f,  // top-right
            0.5f, -0.5f, -0.5f, 1.0f,
            0.0f,  // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f,
            1.0f,  // top-right
            -0.5f, -0.5f, -0.5f, 0.0f,
            0.0f,  // bottom-left
            -0.5f, 0.5f, -0.5f, 0.0f,
            1.0f,  // top-left
            // Front face
            -0.5f, -0.5f, 0.5f, 0.0f,
            0.0f,  // bottom-left
            0.5f, -0.5f, 0.5f, 1.0f,
            0.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 1.0f,
            1.0f,  // top-right
            0.5f, 0.5f, 0.5f, 1.0f,
            1.0f,  // top-right
            -0.5f, 0.5f, 0.5f, 0.0f,
            1.0f,  // top-left
            -0.5f, -0.5f, 0.5f, 0.0f,
            0.0f,  // bottom-left
            // Left face
            -0.5f, 0.5f, 0.5f, 1.0f,
            0.0f,  // top-right
            -0.5f, 0.5f, -0.5f, 1.0f,
            1.0f,  // top-left
            -0.5f, -0.5f, -0.5f, 0.0f,
            1.0f,  // bottom-left
            -0.5f, -0.5f, -0.5f, 0.0f,
            1.0f,  // bottom-left
            -0.5f, -0.5f, 0.5f, 0.0f,
            0.0f,  // bottom-right
            -0.5f, 0.5f, 0.5f, 1.0f,
            0.0f,  // top-right
                   // Right face
            0.5f, 0.5f, 0.5f, 1.0f,
            0.0f,  // top-left
            0.5f, -0.5f, -0.5f, 0.0f,
            1.0f,  // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f,
            1.0f,  // top-right
            0.5f, -0.5f, -0.5f, 0.0f,
            1.0f,  // bottom-right
            0.5f, 0.5f, 0.5f, 1.0f,
            0.0f,  // top-left
            0.5f, -0.5f, 0.5f, 0.0f,
            0.0f,  // bottom-left
            // Bottom face
            -0.5f, -0.5f, -0.5f, 0.0f,
            1.0f,  // top-right
            0.5f, -0.5f, -0.5f, 1.0f,
            1.0f,  // top-left
            0.5f, -0.5f, 0.5f, 1.0f,
            0.0f,  // bottom-left
            0.5f, -0.5f, 0.5f, 1.0f,
            0.0f,  // bottom-left
            -0.5f, -0.5f, 0.5f, 0.0f,
            0.0f,  // bottom-right
            -0.5f, -0.5f, -0.5f, 0.0f,
            1.0f,  // top-right
            // Top face
            -0.5f, 0.5f, -0.5f, 0.0f,
            1.0f,  // top-left
            0.5f, 0.5f, 0.5f, 1.0f,
            0.0f,  // bottom-right
            0.5f, 0.5f, -0.5f, 1.0f,
            1.0f,  // top-right
            0.5f, 0.5f, 0.5f, 1.0f,
            0.0f,  // bottom-right
            -0.5f, 0.5f, -0.5f, 0.0f,
            1.0f,  // top-left
            -0.5f, 0.5f, 0.5f, 0.0f,
            0.0f  // bottom-left
        };
        float planeVertices[] = {
            // positions          // texture Coords (note we set these higher
            // than 1 (together with GL_REPEAT as texture wrapping mode). this
            // will cause the floor texture to repeat)
            5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, 5.0f,  0.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f,

            5.0f, -0.5f, 5.0f, 2.0f, 0.0f, -5.0f, -0.5f, -5.0f, 0.0f, 2.0f, 5.0f,  -0.5f, -5.0f, 2.0f, 2.0f};
        float quadVertices[] = {// vertex attributes for a quad that fills the entire screen in Normalized Device
                                // Coordinates. positions   // texCoords
                                -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                                -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f};
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
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
        // plane VAO
        unsigned int planeVAO, planeVBO;
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
        // screen quad VAO
        unsigned int quadVAO, quadVBO;
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        // skybox VAO
        std::uint32_t skyboxVAO, skyboxVBO;
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // framebuffer configuration
        // -------------------------
        unsigned int framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // create a color attachment texture
        unsigned int textureColorbuffer;
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH,
                              SCR_HEIGHT);  // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  rbo);  // now actually attach it
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually
        // complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::printf("ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // load textures
        // -------------
        Texture cubeTexture{"res/textures/container.jpg", ""};
        Texture floorTexture{"res/textures/metal.png", ""};

        std::vector<std::string> faces = {
            "res/textures/skybox/right.jpg",
            "res/textures/skybox/left.jpg",
            "res/textures/skybox/top.jpg",
            "res/textures/skybox/bottom.jpg",
            "res/textures/skybox/front.jpg",
            "res/textures/skybox/back.jpg",};
        const auto skyboxCubemap = load_cubemap(faces);

        // shader configuration
        // --------------------
        shader->use();
        shader->set_i("texture1", 0);

        shader_skybox->use();
        shader_skybox->set_i("skybox", 0);

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
            glDepthFunc(GL_LEQUAL);

            // make sure we clear the framebuffer's content
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader->use();
            glm::mat4 model = glm::mat4(1.0f);
            glm::mat4 view = camera.get_view_matrix();
            glm::mat4 projection =
                glm::perspective(glm::radians(camera.m_zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            shader->set_mat4("view", view);
            shader->set_mat4("projection", projection);

            shader_skybox->use();
            shader_skybox->set_mat4("view", glm::mat4(glm::mat3(view)));
            shader_skybox->set_mat4("projection", projection);


            // cubes
            shader->use();
            glBindVertexArray(cubeVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, cubeTexture.id);
            model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
            shader->set_mat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
            shader->set_mat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            // floor
            glBindVertexArray(planeVAO);
            glBindTexture(GL_TEXTURE_2D, floorTexture.id);
            shader->set_mat4("model", glm::mat4(1.0f));
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
            // skybox
            shader_skybox->use();
            glDepthMask(GL_FALSE);
            glBindVertexArray(skyboxVAO);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);


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

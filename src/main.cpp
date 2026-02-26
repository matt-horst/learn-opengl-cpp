#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
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

  ShaderBuilder sb;
  try {
    sb._m_vertex_src = readFileToString("res/shaders/vertex_model.glsl");
    sb._m_fragment_src = readFileToString("res/shaders/fragment_model.glsl");

  } catch (const std::runtime_error &e) {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }

  std::unique_ptr<Shader> shader;
  try {
    shader = sb.build();
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

  Model model("res/models/backpack/backpack.obj");

  glEnable(GL_DEPTH_TEST);


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

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();

    // view/projection transformations
    glm::mat4 projection =
        glm::perspective(glm::radians(camera.m_zoom),
                         (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = camera.get_view_matrix();
    shader->set_mat4("projection", projection);
    shader->set_mat4("view", view);

    // render the loaded model
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(
        m, glm::vec3(
               0.0f, 0.0f,
               0.0f)); // translate it down so it's at the center of the scene
    m = glm::scale(
        m,
        glm::vec3(1.0f, 1.0f,
                  1.0f)); // it's a bit too big for our scene, so scale it down
    shader->set_mat4("model", m);
    model.draw(*shader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

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

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.zoom(yoffset);
}

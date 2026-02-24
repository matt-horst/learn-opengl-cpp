#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <cstdlib>

#include "camera.hpp"
#include "shader.hpp"

void framebuffer_size_callback(GLFWwindow *, int, int);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

Camera camera;

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

  ShaderBuilder sb, sb_light;
  try {
    sb._m_vertex_src = readFileToString("res/shaders/vertex.glsl");
    sb._m_fragment_src = readFileToString("res/shaders/fragment.glsl");

    sb_light._m_fragment_src =
        readFileToString("res/shaders/fragment_light.glsl");
    sb_light._m_vertex_src = sb._m_vertex_src;
  } catch (const std::runtime_error &e) {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }

  std::unique_ptr<Shader> shader, light_shader;
  try {
    shader = sb.build();
    light_shader = sb_light.build();
  } catch (const std::runtime_error &e) {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }

  float vertices[] = {
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

      -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  unsigned int lightVAO;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  shader->use();

  glUniform3f(glGetUniformLocation(shader->_m_id, "objectColor"), 1.0f, 0.5f,
              0.31f);
  glUniform3f(glGetUniformLocation(shader->_m_id, "lightColor"), 1.0f, 1.0f,
              1.0f);

  glm::vec3 light_pos(1.2f, 1.0f, 2.0f);

  glEnable(GL_DEPTH_TEST);

  glm::vec3 cube_positions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

  float last_frame = 0.0f;

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

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

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->use();
    glBindVertexArray(VAO);

    const glm::mat4 view = camera.get_view_matrix();
    const glm::mat4 projection =
        glm::perspective(glm::radians(camera.m_zoom),
                         (float)width / (float)height, 0.1f, 100.0f);


    // Light source
    {
      light_shader->use();

      const glm::mat4 model = glm::scale(
          glm::translate(glm::mat4(1.0f), light_pos), glm::vec3(0.2f));

      glUniformMatrix4fv(glGetUniformLocation(light_shader->_m_id, "model"), 1,
                         GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(light_shader->_m_id, "view"), 1, GL_FALSE,
              glm::value_ptr(view));
      glUniformMatrix4fv(glGetUniformLocation(light_shader->_m_id, "projection"), 1,
              GL_FALSE, glm::value_ptr(projection));

      glBindVertexArray(lightVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Cube
    {
      shader->use();

      const glm::mat4 model(1.0f);

      glUniformMatrix4fv(glGetUniformLocation(shader->_m_id, "model"), 1,
                         GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(glGetUniformLocation(shader->_m_id, "view"), 1, GL_FALSE,
              glm::value_ptr(view));
      glUniformMatrix4fv(glGetUniformLocation(shader->_m_id, "projection"), 1,
              GL_FALSE, glm::value_ptr(projection));

      glBindVertexArray(VAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

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

#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <sstream>
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

  ShaderBuilder sb, sb_light;
  try {
    sb._m_vertex_src = readFileToString("res/shaders/vertex.glsl");
    sb._m_fragment_src = readFileToString("res/shaders/fragment.glsl");

    sb_light._m_fragment_src =
        readFileToString("res/shaders/fragment_light.glsl");
    sb_light._m_vertex_src = readFileToString("res/shaders/vertex_light.glsl");
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
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
      0.0f,  -1.0f, 1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
      0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,
      -0.5f, -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
      -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
      -1.0f, 0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,
      -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      1.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindVertexArray(VAO);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // texture attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  unsigned int diffuse_map, specular_map;
  try {
    diffuse_map = load_texture("res/textures/container2.png");
    specular_map = load_texture("res/textures/container2_specular.png");
  } catch (const std::runtime_error &e) {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }

  unsigned int lightVAO;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glm::vec3 light_pos(1.2f, 1.0f, 2.0f);

  glEnable(GL_DEPTH_TEST);

  float last_frame = 0.0f;

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  shader->use();
  shader->set_i("material.diffuse", 0);
  shader->set_i("material.specular", 1);

  const glm::vec3 cube_positions[] = {
      glm::vec3(0.0f, 0.0f, 0.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};

  const glm::vec3 point_light_positions[] = {
      glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
      glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

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

    const glm::mat4 view = camera.get_view_matrix();
    const glm::mat4 projection =
        glm::perspective(glm::radians(camera.m_zoom),
                         (float)width / (float)height, 0.1f, 100.0f);

    // Light source
    {
      light_shader->use();

      for (unsigned int i = 0; i < 4; i++) {
          const glm::mat4 model = glm::scale(
                  glm::translate(glm::mat4(1.0f), point_light_positions[i]), glm::vec3(0.2f));

          light_shader->set_mat4("model", model);
          light_shader->set_mat4("view", view);
          light_shader->set_mat4("projection", projection);

          glBindVertexArray(lightVAO);
          glDrawArrays(GL_TRIANGLES, 0, 36);
      }
    }

    for (unsigned int i = 0; i < 10; i++) {
      // Cube
      {
        const glm::mat4 model =
            glm::rotate(glm::translate(glm::mat4(1.0f), cube_positions[i]),
                        glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));

        shader->use();

        shader->set_mat4("model", model);
        shader->set_mat4("view", view);
        shader->set_mat4("projection", projection);
        shader->set_vec3("viewPos", camera.m_position);

        // Light properties
        // Directional
        shader->set_vec3("dirLight.ambient", 0.2f, 0.2f, 0.2f);
        shader->set_vec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
        shader->set_vec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
        shader->set_vec3("dirLight.direction", -0.2f, -1.0f, -0.3f);

        // Point Lights
        for (unsigned int i = 0; i < 4; i++) {
          std::stringstream ss;
          ss << "pointLights[" << i << "].";
          const std::string prefix = ss.str();
          shader->set_vec3(prefix + "position", cube_positions[i]);
          shader->set_vec3(prefix + "ambient", 0.05f, 0.05f, 0.05f);
          shader->set_vec3(prefix + "diffuse", 0.8f, 0.8f, 0.8f);
          shader->set_vec3(prefix + "specular", 1.0f, 1.0f, 1.0f);
          shader->set_f(prefix + "constant", 1.0f);
          shader->set_f(prefix + "linear", 0.09f);
          shader->set_f(prefix + "quadratic", 0.032f);
        }

        // Spot Light
        shader->set_vec3("spotLight.position", camera.m_position);
        shader->set_vec3("spotLight.direction", camera.m_front);
        shader->set_vec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        shader->set_vec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        shader->set_vec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        shader->set_f("spotLight.constant", 1.0f);
        shader->set_f("spotLight.constant", 0.09f);
        shader->set_f("spotLight.constant", 0.032f);
        shader->set_f("spotLight.innerCutOff", glm::cos(glm::radians(12.5f)));
        shader->set_f("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));

        // Material properties
        shader->set_i("material.diffuse", 0);
        shader->set_i("material.specular", 1);
        shader->set_f("material.shininess", 64.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_map);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular_map);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
      }
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

unsigned int load_texture(const std::string &path) {
  unsigned int id;
  glGenTextures(1, &id);

  int width, height, k;
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &k, 0);

  if (data == NULL) {
    throw std::runtime_error("couldn't load image file: " + path);
  }

  GLenum format;
  if (k == 1) {
    format = GL_RED;
  } else if (k == 3) {
    format = GL_RGB;
  } else if (k == 4) {
    format = GL_RGBA;
  } else {
    throw std::runtime_error("Unable to infer texture format");
  }

  glBindTexture(GL_TEXTURE_2D, id);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);

  return id;
}

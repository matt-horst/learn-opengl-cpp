#include "shader.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

void Shader::use() { glUseProgram(this->_m_id); }

std::string readFileToString(const std::string &filename) {
  // Open the file for reading. Use std::ios_base::binary for binary files
  std::ifstream ifs(filename, std::ios_base::in);

  if (!ifs.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  // Use string constructor with stream iterators to read the whole file
  std::string content = std::string(std::istreambuf_iterator<char>(ifs),
                                    std::istreambuf_iterator<char>());

  // Optional: Check for read errors (excluding eof, which is expected)
  if (!ifs && !ifs.eof()) {
    throw std::runtime_error("Failed to read file: " + filename);
  }

  return content;
}

std::unique_ptr<Shader> ShaderBuilder::build() {
  int success;
  const int buff_size = 512;
  char infolog[buff_size] = {};

  const unsigned int id = glCreateProgram();

  const unsigned int vertex_id = glCreateShader(GL_VERTEX_SHADER);
  const char *vs = _m_vertex_src.c_str();
  glShaderSource(vertex_id, 1, &vs, NULL);
  glCompileShader(vertex_id);

  glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(vertex_id, buff_size, NULL, infolog);

    throw std::runtime_error("Failed to compile vertex shader: " +
                             std::string(infolog));
  }

  const unsigned int fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
  const char *fs = _m_fragment_src.c_str();
  glShaderSource(fragment_id, 1, &fs, NULL);
  glCompileShader(fragment_id);

  glGetShaderiv(fragment_id, GL_COMPILE_STATUS, &success);

  if (!success) {
    const int buff_size = 512;
    char infolog[buff_size] = {};
    glGetShaderInfoLog(fragment_id, buff_size, NULL, infolog);

    throw std::runtime_error("Failed to compile fragment shader: " +
                             std::string(infolog));
  }

  glAttachShader(id, vertex_id);
  glAttachShader(id, fragment_id);
  glLinkProgram(id);

  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(id, buff_size, NULL, infolog);
    throw std::runtime_error("Failed to link shader program: " +
                             std::string(infolog));
  }

  glDeleteShader(vertex_id);
  glDeleteShader(fragment_id);

  return std::make_unique<Shader>(id);
}

Shader::Shader(unsigned int id) : _m_id(id) {}

Shader::~Shader() {
  glDeleteProgram(this->_m_id);
}

void Shader::set_vec3(const std::string &name, glm::vec3 v) const {
  glUniform3f(glGetUniformLocation(_m_id, name.c_str()), v.x, v.y, v.z);
}

void Shader::set_vec3(const std::string &name, float x, float y,
                      float z) const {
  glUniform3f(glGetUniformLocation(_m_id, name.c_str()), x, y, z);
}

void Shader::set_vec3(const std::string &name, float v) const {
  glUniform3f(glGetUniformLocation(_m_id, name.c_str()), v, v, v);
}

void Shader::set_mat4(const std::string &name, glm::mat4 v) const {
  glUniformMatrix4fv(glGetUniformLocation(_m_id, name.c_str()), 1, GL_FALSE,
                     glm::value_ptr(v));
}

void Shader::set_f(const std::string &name, float v) const {
  glUniform1f(glGetUniformLocation(_m_id, name.c_str()), v);
}

void Shader::set_i(const std::string &name, int v) const {
  glUniform1i(glGetUniformLocation(_m_id, name.c_str()), v);
}

#include "shader.hpp"
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

  return std::make_unique<Shader>(Shader(id));
}

Shader::Shader(unsigned int id) : _m_id(id) {}

Shader::~Shader() { /* glDeleteProgram(this->_m_id); */ }

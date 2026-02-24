#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <memory>
#include <string>

std::string readFileToString(const std::string &filename);

class Shader {
public:
  unsigned int _m_id;

  void use();
  Shader(unsigned int id);
  ~Shader();

  void set_vec3(const std::string &name, glm::vec3 v);
  void set_vec3(const std::string &name, float x, float y, float z);
  void set_vec3(const std::string &name, float x);

  void set_mat4(const std::string &name, glm::mat4 v);

  void set_f(const std::string &name, float v);
  void set_i(const std::string &name, int v);

private:
};

class ShaderBuilder {
public:
  std::string _m_vertex_src;
  std::string _m_fragment_src;

  std::unique_ptr<Shader> build();

private:
};

#endif

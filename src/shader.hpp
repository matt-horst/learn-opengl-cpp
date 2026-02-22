#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>

#include <memory>
#include <string>

std::string readFileToString(const std::string &filename);

class Shader {
public:
  unsigned int _m_id;

  void use();
  Shader(unsigned int id);
  ~Shader();
};

class ShaderBuilder {
public:
  std::string _m_vertex_src;
  std::string _m_fragment_src;

  std::unique_ptr<Shader> build();

private:
};

#endif

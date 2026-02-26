#ifndef MESH_HPP
#define MESH_HPP

#include "shader.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
  glm::vec3 position, normal;
  glm::vec2 tex_coord;
};

struct Texture {
  unsigned int id;
  std::string type;
  std::string file_path;

  Texture(const std::string &file_path, const std::string &type);
};

class Mesh {
public:
  // Mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  Mesh(const std::string &name, std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures);

  void draw(const Shader &shader) const;

  const std::string name;

private:
  unsigned int vao, vbo, ebo;
  void setup_mesh();
};

#endif

#ifndef MESH_HPP
#define MESH_HPP

#include "shader.hpp"

#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Vertex {
  glm::vec3 position, normal, tex_coord;
};

struct Texture {
  unsigned int id;
  std::string type; // TODO: Maybe enum
};

class Mesh {
public:
  // Mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures);

  void draw(const Shader &shader);

private:
  unsigned int vao, vbo, ebo;
  void setup_mesh();
};

#endif

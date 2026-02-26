#include "mesh.hpp"
#include "shader.hpp"
#include <cstdio>
#include <stb/stb_image.h>
#include <stdexcept>

Mesh::Mesh(const std::string &name, std::vector<Vertex> vertices,
           std::vector<unsigned int> indices, std::vector<Texture> textures)
    : vertices(vertices), indices(indices), textures(textures), name(name) {
  setup_mesh();
}

void Mesh::draw(const Shader &shader) const {
  unsigned int diffuse_n = 1;
  unsigned int specular_n = 1;

  for (unsigned int i = 0; i < textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);

    std::string number;
    std::string name = textures[i].type;
    if (name == "texture_diffuse") {
      number = std::to_string(diffuse_n++);
    } else if (name == "texture_specular") {
      number = std::to_string(specular_n++);
    } else {
      throw std::runtime_error("Unknown texture type: " + name);
    }

    shader.set_i((name + number).c_str(), i);
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }

  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE0);
}

void Mesh::setup_mesh() {
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // Vertex positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  // Vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, normal));

  // Texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, tex_coord));

  glBindVertexArray(0);
}

Texture::Texture(const std::string &file_path, const std::string &type)
    : type(type), file_path(file_path) {
  glGenTextures(1, &id);

  int width, height, k;
  unsigned char *data = stbi_load(file_path.c_str(), &width, &height, &k, 0);

  if (data == NULL) {
    throw std::runtime_error("couldn't load image file: " + file_path);
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
}

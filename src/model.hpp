#ifndef MODEL_HPP
#define MODEL_HPP

#include "assimp/material.h"
#include "mesh.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <string>
#include <vector>

class Model {
public:
  Model(const std::string &file_path);
  void draw(const Shader &shader);

private:
  std::vector<Mesh> meshes;
  std::string directory;

  void load_model(const std::string &file_path);
  void process_node(aiNode *node, const aiScene *scene);
  Mesh process_mesh(aiMesh *mesh, const aiScene *scene);
  std::vector<Texture> load_material_textures(const aiMaterial *mat,
                                              aiTextureType type,
                                              std::string type_name);
};

#endif

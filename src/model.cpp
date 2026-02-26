#include "model.hpp"
#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "glm/fwd.hpp"
#include "shader.hpp"
#include <cstdio>
#include <stdexcept>

std::vector<Texture> loaded_textures;

Model::Model(const std::string &file_path)
    : directory(file_path.substr(0, file_path.find_last_of('/'))) {
  load_model(file_path);
}

void Model::draw(const Shader &shader) {
  for (const auto &mesh : meshes) {
    mesh.draw(shader);
  }
}

void Model::load_model(const std::string &file_path) {
  Assimp::Importer importer;

  const aiScene *scene =
      importer.ReadFile(file_path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error(std::string("Assimp error: ") +
                             importer.GetErrorString());
  }

  process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode *node, const aiScene *scene) {
  std::printf("processing node: %s\n", node->mName.C_Str());
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(process_mesh(mesh, scene));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene);
  }
}

Mesh Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // Process the vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;

    const auto &vp = mesh->mVertices[i];
    vertex.position = glm::vec3(vp.x, vp.y, vp.z);

    const auto &vn = mesh->mNormals[i];
    vertex.normal = glm::vec3(vn.x, vn.y, vn.z);

    if (mesh->mTextureCoords[0]) {
      const auto &vtc = mesh->mTextureCoords[0][i];
      vertex.tex_coord = glm::vec2(vtc.x, vtc.y);
    } else {
      vertex.tex_coord = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  // Process the indices
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    const auto &face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  // Process the textures
  if (mesh->mMaterialIndex >= 0) {
    const aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuse_maps = load_material_textures(
        material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    std::vector<Texture> specular_maps = load_material_textures(
        material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
  }

  return Mesh(mesh->mName.C_Str(), vertices, indices, textures);
}

std::vector<Texture> Model::load_material_textures(const aiMaterial *mat,
                                                   aiTextureType type,
                                                   std::string type_name) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);

    const std::string file_name = directory + "/" + std::string(str.C_Str());

    bool skip = false;
    for (const auto &tex : loaded_textures) {
      if (tex.file_path == file_name) {
        textures.push_back(tex);
        skip = true;
        break;
      }
    }

    if (!skip) {
      Texture tex(file_name, type_name);
      textures.push_back(tex);
      loaded_textures.push_back(tex);
    }
  }

  return textures;
}

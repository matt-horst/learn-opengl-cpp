#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

std::string readFileToString(const std::string &filename) {
  // Open the file for reading. Use std::ios_base::binary for binary files
  std::ifstream ifs(filename, std::ios_base::in);

  if (!ifs.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  // Use string constructor with stream iterators to read the whole file
  std::string content(std::istreambuf_iterator<char>(ifs),
                      std::istreambuf_iterator<char>());

  // Optional: Check for read errors (excluding eof, which is expected)
  if (!ifs && !ifs.eof()) {
    throw std::runtime_error("Failed to read file: " + filename);
  }

  return content;
}

class Shader {};

class ShaderBuilder {
public:
  std::string vertex_src;
  std::string fragment_src;

  Shader build();

private:
};

#endif

#include "shader.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"

void Shader::use() const { glUseProgram(this->m_id); }

std::string readFileToString(const std::string& filename) {
    // Open the file for reading. Use std::ios_base::binary for binary files
    std::ifstream ifs(filename, std::ios_base::in);

    if (!ifs.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    // Use string constructor with stream iterators to read the whole file
    std::string content = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

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
    const char* vs = m_vertex_src.c_str();
    glShaderSource(vertex_id, 1, &vs, NULL);
    glCompileShader(vertex_id);

    glGetShaderiv(vertex_id, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(vertex_id, buff_size, NULL, infolog);

        throw std::runtime_error("Failed to compile vertex shader: " + std::string(infolog));
    }

    if (!m_geometry_src.empty()) {
        const char *src = m_geometry_src.c_str();
        const unsigned int shader_id = compile_shader(GL_GEOMETRY_SHADER, src);
        glAttachShader(id, shader_id);
        glDeleteShader(shader_id);
    }

    if (!m_vertex_src.empty()) {
        const char *src = m_vertex_src.c_str();
        const unsigned int shader_id = compile_shader(GL_VERTEX_SHADER, src);
        glAttachShader(id, shader_id);
        glDeleteShader(shader_id);
    }

    if (!m_fragment_src.empty()) {
        const char *src = m_fragment_src.c_str();
        const unsigned int shader_id = compile_shader(GL_FRAGMENT_SHADER, src);
        glAttachShader(id, shader_id);
        glDeleteShader(shader_id);
    }

    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, buff_size, NULL, infolog);
        throw std::runtime_error("Failed to link shader program: " + std::string(infolog));
    }

    return std::make_unique<Shader>(id);
}

unsigned int ShaderBuilder::compile_shader(unsigned int type, const char *src) {
    const unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    int ok;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        const int buf_size = 512;
        char buf[buf_size];
        glGetShaderInfoLog(id, buf_size, NULL, buf);
        throw std::runtime_error("failed to compile shader program: " + std::string(buf));
    }

    return id;
}

Shader::Shader(unsigned int id) : m_id(id) {}

Shader::~Shader() { glDeleteProgram(this->m_id); }

void Shader::set_vec3(const std::string& name, glm::vec3 v) const {
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), v.x, v.y, v.z);
}

void Shader::set_vec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), x, y, z);
}

void Shader::set_vec3(const std::string& name, float v) const {
    glUniform3f(glGetUniformLocation(m_id, name.c_str()), v, v, v);
}

void Shader::set_vec2(const std::string& name, glm::vec2 v) const {
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), v.x, v.y);
}

void Shader::set_vec2(const std::string& name, float x, float y) const {
    glUniform2f(glGetUniformLocation(m_id, name.c_str()), x, y);
}

void Shader::set_mat4(const std::string& name, glm::mat4 v) const {
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(v));
}

void Shader::set_f(const std::string& name, float v) const {
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), v);
}

void Shader::set_i(const std::string& name, int v) const { glUniform1i(glGetUniformLocation(m_id, name.c_str()), v); }


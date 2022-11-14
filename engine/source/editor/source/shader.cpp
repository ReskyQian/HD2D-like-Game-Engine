#include "editor/include/shader.h"
// #include "editor/include/texture2d.h"

#include <string>
#include <string_view>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

Shader::Shader(std::string_view file_path) : id_{0}
{
    std::ifstream fs{};
    fs.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        fs.open(std::string{file_path});
        std::stringstream ss{};
        ss << fs.rdbuf();
        fs.close();
        source_ = ss.str();
    }
    catch (std::ifstream::failure e) {
        std::cout << "Error::Shader::File_Not_Successfully_Read" << std::endl;
    }
}

Shader::~Shader() {
    if (id_ != 0)
        glDeleteShader(id_);
}

VertexShader::VertexShader(std::string_view file_path)
: Shader { file_path } 
{
    id_ = glCreateShader(GL_VERTEX_SHADER);
    auto source_str = source_.c_str();
    glShaderSource(id_, 1, &source_str, nullptr);
    glCompileShader(id_);

    int success{};
    char log_info[512];
    glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(id_, 512, nullptr, log_info);
        std::cout << "ERROR:SHADER::VERTEX::COMPILEATION_FAILED\n" << log_info << std::endl;
    }
}

FragmentShader::FragmentShader(std::string_view file_path)
: Shader { file_path } 
{
    id_ = glCreateShader(GL_FRAGMENT_SHADER);
    auto source_str = source_.c_str();
    glShaderSource(id_, 1, &source_str, nullptr);
    glCompileShader(id_);

    int success{};
    char log_info[512];
    glGetShaderiv(id_, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(id_, 512, nullptr, log_info);
        std::cout << "ERROR:SHADER::FRAGMENT::COMPILEATION_FAILED\n" << log_info << std::endl;
    }
}

ShaderProgram::ShaderProgram(std::string_view vertex_shader, std::string_view fragment_shader)
: id_{0}
{
    VertexShader vertex {vertex_shader};
    FragmentShader fragment {fragment_shader};

    id_ = glCreateProgram();
    glAttachShader(id_, vertex.get_id());
    glAttachShader(id_, fragment.get_id());
    glLinkProgram(id_);

    int success{};
    char log_info[512];
    glGetProgramiv(id_, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id_, 512, nullptr, log_info);
        std::cout << "ERROR:SHADER::PROGRAM::LINK_FAILED\n" << log_info << std::endl;
    }
}

ShaderProgram::~ShaderProgram() {
    if (id_ != 0) {
        glDeleteProgram(id_);
    }
}

void ShaderProgram::set_uniform(std::string_view name, bool value) const noexcept
{
    glUniform1i(glGetUniformLocation(id_, name.data()), static_cast<int>(value));
}

void ShaderProgram::set_uniform(std::string_view name, int value) const noexcept
{
    glUniform1i(glGetUniformLocation(id_, name.data()), value);
}

void ShaderProgram::set_uniform(std::string_view name, float value) const noexcept
{
    glUniform1f(glGetUniformLocation(id_, name.data()), value);
}

void ShaderProgram::set_uniform(std::string_view name, glm::mat4 value) const noexcept
{
    glUniformMatrix4fv(glGetUniformLocation(id_, name.data()), 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::set_texture(std::string_view name, int value) const noexcept
{
    ShaderProgram::set_uniform(name, value);
}

void ShaderProgram::use() const noexcept
{
    glUseProgram(id_);
}
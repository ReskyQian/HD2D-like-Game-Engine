#ifndef _SHADER_H__
#define _SHADER_H__

#include <string>
#include <glm/mat4x4.hpp>

class Shader {
public:
    explicit Shader(std::string_view file_path);
    // Shader(const Shader&) = delete;
    // Shader& operator=(const Shader&) = delete;

    // Shader(Shader&& other) noexcept = default;
    // Shader& operator=(Shader&& other) noexcept = default;

    virtual ~Shader();

    constexpr unsigned getId() const noexcept {return id_;}

protected:
    unsigned id_;
    std::string source_;
};

class VertexShader : public Shader {
public:
    explicit VertexShader(std::string_view file_path);
};

class FragmentShader : public Shader {
public:
    explicit FragmentShader(std::string_view file_path);
};

class GeometryShader : public Shader {
public:
    explicit GeometryShader(std::string_view file_path);
};

class ShaderProgram {
public:
    ShaderProgram(std::string_view vertex_shader, 
                  std::string_view fragment_shader);
    ShaderProgram(std::string_view vertex_shader, 
                  std::string_view geometry_shader,
                  std::string_view fragment_shader);

    ~ShaderProgram();

    void use() const noexcept;

    void setUniform(const std::string_view name, bool value) const noexcept;
    void setUniform(const std::string_view name, int value) const noexcept;
    void setUniform(const std::string_view name, unsigned int value) const noexcept;
    void setUniform(const std::string_view name, float value) const noexcept;
    void setUniform(const std::string_view name, const glm::vec3& value) const noexcept;
    void setUniform(const std::string_view name, const glm::mat4& value) const noexcept;

    void setTexture(std::string_view name, int value) const noexcept;

    void setUniformBlock(std::string_view name, int value) const noexcept;

private:
    unsigned id_;
};


#endif // _SHADER_H__
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>

class SDFShader
{
public:
    SDFShader(const char* vertexPath, const char* fragmentPath);

    void Use() const;

    void Reload() const;

    GLint GetUniformLocation(const char* name) const;
    unsigned int GetUniformIndex(GLint location) const;
    unsigned int GetUniformIndex(const char* name) const;


    void SetUniform(const char* name, const void* data) const;

    unsigned int GetUniformCount() const;
    void SetUniform(unsigned int index, const void* data) const;
    void GetUniform(unsigned int index, void* data) const;
    unsigned int GetUniformSize(unsigned int index) const;

    template<typename T>
    bool ValidateUniform(unsigned int index) const;

    static bool ReadShaderFile(const char* path, std::string &shaderCode);

private:

    typedef std::function<void(GLuint, GLint, void* value)> GetFunction;
    typedef std::function<void(GLint, const void* value)> SetFunction;

    struct Uniform
    {
        Uniform() : type(GL_INVALID_ENUM), location(-1) {}
        GLenum type;
        GLint location;
        GetFunction getFn;
        SetFunction setFn;
    };

    void LoadActiveUniforms();

    void GetTypeInfo(GLenum type, unsigned int& size) const;
    void GetTypeInfo(GLenum type, unsigned int& size, GetFunction& getFn, SetFunction& setFn) const;

    template<typename T>
    void GetTypeInfo(GLenum& type, unsigned int& size, GetFunction& getFn, SetFunction& setFn) const;

    static bool CheckShaderErrors(GLuint shader);

    mutable GLuint m_Program;

    std::vector<Uniform> m_Uniforms;
    std::unordered_map<GLint, unsigned int> m_LocationUniforms;

    std::string m_VertexPath;
    std::string m_FragmentPath;

    mutable std::size_t m_VertexHash;
    mutable std::size_t m_FragmentHash;
};

template<typename T>
bool SDFShader::ValidateUniform(unsigned int index) const
{
    GLenum type;
    unsigned int size;
    GetFunction getFn;
    SetFunction setFn;
    GetTypeInfo<T>(type, size, getFn, setFn);

    const Uniform& uniform = m_Uniforms[index];
    return uniform.type == type || (type == GL_INT && uniform.type >= GL_SAMPLER_1D && uniform.type <= GL_SAMPLER_CUBE);
}


#define GetTypeInfoSpecializationDeclaration(T)\
template<> \
void SDFShader::GetTypeInfo<T>(GLenum &type, unsigned int& size, GetFunction& getFn, SetFunction& setFn) const

GetTypeInfoSpecializationDeclaration(GLfloat);
GetTypeInfoSpecializationDeclaration(glm::vec2);
GetTypeInfoSpecializationDeclaration(glm::vec3);
GetTypeInfoSpecializationDeclaration(glm::vec4);

GetTypeInfoSpecializationDeclaration(GLint);
GetTypeInfoSpecializationDeclaration(glm::ivec2);
GetTypeInfoSpecializationDeclaration(glm::ivec3);
GetTypeInfoSpecializationDeclaration(glm::ivec4);

GetTypeInfoSpecializationDeclaration(GLuint);
GetTypeInfoSpecializationDeclaration(glm::uvec2);
GetTypeInfoSpecializationDeclaration(glm::uvec3);
GetTypeInfoSpecializationDeclaration(glm::uvec4);

GetTypeInfoSpecializationDeclaration(glm::mat2);
GetTypeInfoSpecializationDeclaration(glm::mat2x3);
GetTypeInfoSpecializationDeclaration(glm::mat2x4);

GetTypeInfoSpecializationDeclaration(glm::mat3);
GetTypeInfoSpecializationDeclaration(glm::mat3x2);
GetTypeInfoSpecializationDeclaration(glm::mat3x4);

GetTypeInfoSpecializationDeclaration(glm::mat4);
GetTypeInfoSpecializationDeclaration(glm::mat4x2);
GetTypeInfoSpecializationDeclaration(glm::mat4x3);

#include "Shader.h"

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "RayTracer.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
    : m_Program(0)
    , m_VertexPath(vertexPath), m_FragmentPath(fragmentPath)
    , m_VertexHash(0), m_FragmentHash(0)
{
    Reload();

    if (m_Program)
    {
        LoadActiveUniforms();
    }
}

void Shader::Use() const
{
    glUseProgram(m_Program);
}

void Shader::Reload() const
{
    std::string vertexShaderCode, fragmentShaderCode;

    bool vertexRead = ReadShaderFile(m_VertexPath.c_str(), vertexShaderCode);
    bool fragmentRead = ReadShaderFile(m_FragmentPath.c_str(), fragmentShaderCode);

    if (vertexRead && fragmentRead)
    {
        std::size_t prevVertexHash = m_VertexHash;
        m_VertexHash = std::hash<std::string>{}(vertexShaderCode);

        std::size_t prevFragmentHash = m_FragmentHash;
        m_FragmentHash = std::hash<std::string>{}(fragmentShaderCode);

        // No need to recompile if files didn't change
        if (!RayTracer::HasSourceChanged() && m_VertexHash == prevVertexHash && m_FragmentHash == prevFragmentHash)
            return;

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char* vertexSources[] = { vertexShaderCode.c_str() };
        glShaderSource(vertexShader, 1, vertexSources, nullptr);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char* fragmentSources[] = { RayTracer::GetLibrarySource(), fragmentShaderCode.c_str(), RayTracer::GetRayTracerSource() };
        glShaderSource(fragmentShader, 3, fragmentSources, nullptr);
        glCompileShader(fragmentShader);

        GLuint program = 0;
        if (CheckShaderErrors(vertexShader) & CheckShaderErrors(fragmentShader))
        {
            program = glCreateProgram();
            glAttachShader(program, vertexShader);
            glAttachShader(program, fragmentShader);

            glLinkProgram(program);
            if (!CheckShaderErrors(program))
            {
                glDeleteProgram(m_Program);
                program = 0;
            }
        }

        if (program)
        {
            glDeleteProgram(m_Program);
            m_Program = program;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
}

bool Shader::ReadShaderFile(const char* path, std::string& shaderCode)
{
    bool succeded = false;
    std::ifstream shaderStream(path);
    if (shaderStream)
    {
        std::ostringstream stringStream;
        stringStream << shaderStream.rdbuf();
        shaderCode = stringStream.str();
        succeded = true;
    }
    else
    {
        std::cout << "ERROR:: Can't find file: " << path << std::endl;
    }

    return succeded;
}

bool Shader::CheckShaderErrors(GLuint shader)
{
    GLint success;

    GLchar infoLog[1024];
    const char* errorType = nullptr;

    if (glIsProgram(shader))
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            errorType = "LINKING SHADERS";
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
        }
    }
    else
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            GLint shaderType;
            glGetShaderiv(shader, GL_SHADER_TYPE, &shaderType);
            errorType = shaderType == GL_VERTEX_SHADER ? "COMPILING VERTEX SHADER" : "COMPILING FRAGMENT SHADER";
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
        }
    }

    if (errorType)
    {
        std::cout << "ERROR::" << errorType << ":\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    return success == GL_TRUE;
}


GLint Shader::GetUniformLocation(const char* name) const
{
    int location = -1;
    if (m_Program)
    {
        location = glGetUniformLocation(m_Program, name);
    }
    return location;
}

unsigned int Shader::GetUniformIndex(GLint location) const
{
    return m_LocationUniforms.find(location)->second;
}

unsigned int Shader::GetUniformIndex(const char* name) const
{
    unsigned int index = ~0u;
    int location = GetUniformLocation(name);
    if (location != -1)
    {
        index = GetUniformIndex(location);
    }
    return index;
}


void Shader::SetUniform(const char* name, const void* data) const
{
    SetUniform(GetUniformIndex(name), data);
}



unsigned int Shader::GetUniformCount() const
{
    return (unsigned int)m_Uniforms.size();
}

void Shader::SetUniform(unsigned int index, const void* data) const
{
    if (m_Program && index != ~0u)
    {
        const Uniform& uniform = m_Uniforms[index];
        uniform.setFn(uniform.location, data);
    }
}

void Shader::GetUniform(unsigned int index, void* data) const
{
    if (m_Program && index != ~0u)
    {
        const Uniform& uniform = m_Uniforms[index];
        uniform.getFn(m_Program, uniform.location, data);
    }
}

unsigned int Shader::GetUniformSize(unsigned int index) const
{
    const Uniform& uniform = m_Uniforms[index];

    unsigned int size;
    GetTypeInfo(uniform.type, size);

    return size;
}


void Shader::LoadActiveUniforms()
{
    GLint maxNameLength;
    glGetProgramiv(m_Program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameLength);
    GLchar* nameBuffer = new GLchar[maxNameLength];

    m_LocationUniforms.clear();

    GLint uniformCount;
    glGetProgramiv(m_Program, GL_ACTIVE_UNIFORMS, &uniformCount);

    std::vector<Uniform> uniforms(uniformCount);
    for (int i = 0; i < uniformCount; ++i)
    {
        Uniform& uniform = uniforms[i];
        GLint count;
        glGetActiveUniform(m_Program, i, maxNameLength, nullptr, &count, &uniform.type, nameBuffer);
        assert(count == 1);

        uniform.location = glGetUniformLocation(m_Program, nameBuffer);

        unsigned int typeSize;
        GetTypeInfo(uniform.type, typeSize, uniform.getFn, uniform.setFn);

        m_LocationUniforms[uniform.location] = i;
    }
    m_Uniforms.swap(uniforms);

    delete[] nameBuffer;
}

void Shader::GetTypeInfo(GLenum type, unsigned int& size) const
{
    GetFunction getFn;
    SetFunction setFn;
    GetTypeInfo(type, size, getFn, setFn);
}

#define GetTypeInfoSpecializationConvert(T) GetTypeInfo<T>(innerType, size, getFn, setFn); break
void Shader::GetTypeInfo(GLenum type, unsigned int& size, GetFunction& getFn, SetFunction& setFn) const
{
    GLenum innerType;
    switch (type)
    {
    case GL_FLOAT:
        GetTypeInfoSpecializationConvert(GLfloat);
    case GL_FLOAT_VEC2:
        GetTypeInfoSpecializationConvert(glm::vec2);
    case GL_FLOAT_VEC3:
        GetTypeInfoSpecializationConvert(glm::vec3);
    case GL_FLOAT_VEC4:
        GetTypeInfoSpecializationConvert(glm::vec4);
    case GL_INT:
        GetTypeInfoSpecializationConvert(GLint);
    case GL_INT_VEC2:
        GetTypeInfoSpecializationConvert(glm::ivec2);
    case GL_INT_VEC3:
        GetTypeInfoSpecializationConvert(glm::ivec3);
    case GL_INT_VEC4:
        GetTypeInfoSpecializationConvert(glm::ivec4);
    case GL_UNSIGNED_INT:
        GetTypeInfoSpecializationConvert(GLuint);
    case GL_UNSIGNED_INT_VEC2:
        GetTypeInfoSpecializationConvert(glm::uvec2);
    case GL_UNSIGNED_INT_VEC3:
        GetTypeInfoSpecializationConvert(glm::uvec3);
    case GL_UNSIGNED_INT_VEC4:
        GetTypeInfoSpecializationConvert(glm::uvec4);
    case GL_FLOAT_MAT2:
        GetTypeInfoSpecializationConvert(glm::mat2);
    case GL_FLOAT_MAT2x3:
        GetTypeInfoSpecializationConvert(glm::mat2x3);
    case GL_FLOAT_MAT2x4:
        GetTypeInfoSpecializationConvert(glm::mat2x4);
    case GL_FLOAT_MAT3:
        GetTypeInfoSpecializationConvert(glm::mat3);
    case GL_FLOAT_MAT3x2:
        GetTypeInfoSpecializationConvert(glm::mat3x2);
    case GL_FLOAT_MAT3x4:
        GetTypeInfoSpecializationConvert(glm::mat3x4);
    case GL_FLOAT_MAT4:
        GetTypeInfoSpecializationConvert(glm::mat4);
    case GL_FLOAT_MAT4x2:
        GetTypeInfoSpecializationConvert(glm::mat4x2);
    case GL_FLOAT_MAT4x3:
        GetTypeInfoSpecializationConvert(glm::mat4x3);
    case GL_SAMPLER_1D:
        GetTypeInfoSpecializationConvert(GLint);
    case GL_SAMPLER_2D:
        GetTypeInfoSpecializationConvert(GLint);
    case GL_SAMPLER_3D:
        GetTypeInfoSpecializationConvert(GLint);
    case GL_SAMPLER_CUBE:
        GetTypeInfoSpecializationConvert(GLint);
    }
}


#define GetTypeInfoSpecializationDefinition(T, GLType, STATIC_FUNCTIONS)\
template<> \
void Shader::GetTypeInfo<T>(GLenum &type, unsigned int& size, GetFunction& getFn, SetFunction& setFn) const \
{ \
    STATIC_FUNCTIONS; \
    type = GLType; \
    size = sizeof(T); \
    getFn = staticGetFn; \
    setFn = staticSetFn; \
}

#define UNIFORM_FUNCTIONS_VECTOR(T, t, n) \
    static GetFunction staticGetFn = [](GLuint program, GLint location, void* value) { glGetUniform##t##v(program, location, reinterpret_cast<T*>(value)); }; \
    static SetFunction staticSetFn = [](GLint location, const void* value) { glUniform##n##t##v(location, 1, reinterpret_cast<const T*>(value)); };

#define UNIFORM_FUNCTIONS_MATRIX(n, m) \
    static GetFunction staticGetFn = [](GLuint program, GLint location, void* value) { glGetUniformfv(program, location, reinterpret_cast<GLfloat*>(value)); }; \
    static SetFunction staticSetFn = [](GLint location, const void* value) { glUniformMatrix##n##x##m##fv(location, 1, false, reinterpret_cast<const GLfloat*>(value)); };

#define UNIFORM_FUNCTIONS_MATRIX_SQUARE(n) \
    static GetFunction staticGetFn = [](GLuint program, GLint location, void* value) { glGetUniformfv(program, location, reinterpret_cast<GLfloat*>(value)); }; \
    static SetFunction staticSetFn = [](GLint location, const void* value) { glUniformMatrix##n##fv(location, 1, false, reinterpret_cast<const GLfloat*>(value)); };

GetTypeInfoSpecializationDefinition(GLfloat, GL_FLOAT, UNIFORM_FUNCTIONS_VECTOR(GLfloat, f, 1))
GetTypeInfoSpecializationDefinition(glm::vec2, GL_FLOAT_VEC2, UNIFORM_FUNCTIONS_VECTOR(GLfloat, f, 2))
GetTypeInfoSpecializationDefinition(glm::vec3, GL_FLOAT_VEC3, UNIFORM_FUNCTIONS_VECTOR(GLfloat, f, 3))
GetTypeInfoSpecializationDefinition(glm::vec4, GL_FLOAT_VEC4, UNIFORM_FUNCTIONS_VECTOR(GLfloat, f, 4))

GetTypeInfoSpecializationDefinition(GLint, GL_INT, UNIFORM_FUNCTIONS_VECTOR(GLint, i, 1))
GetTypeInfoSpecializationDefinition(glm::ivec2, GL_INT_VEC2, UNIFORM_FUNCTIONS_VECTOR(GLint, i, 2))
GetTypeInfoSpecializationDefinition(glm::ivec3, GL_INT_VEC3, UNIFORM_FUNCTIONS_VECTOR(GLint, i, 3))
GetTypeInfoSpecializationDefinition(glm::ivec4, GL_INT_VEC4, UNIFORM_FUNCTIONS_VECTOR(GLint, i, 4))

GetTypeInfoSpecializationDefinition(GLuint, GL_UNSIGNED_INT, UNIFORM_FUNCTIONS_VECTOR(GLuint, ui, 1))
GetTypeInfoSpecializationDefinition(glm::uvec2, GL_UNSIGNED_INT_VEC2, UNIFORM_FUNCTIONS_VECTOR(GLuint, ui, 2))
GetTypeInfoSpecializationDefinition(glm::uvec3, GL_UNSIGNED_INT_VEC3, UNIFORM_FUNCTIONS_VECTOR(GLuint, ui, 3))
GetTypeInfoSpecializationDefinition(glm::uvec4, GL_UNSIGNED_INT_VEC4, UNIFORM_FUNCTIONS_VECTOR(GLuint, ui, 4))

GetTypeInfoSpecializationDefinition(glm::mat2, GL_FLOAT_MAT2, UNIFORM_FUNCTIONS_MATRIX_SQUARE(2))
GetTypeInfoSpecializationDefinition(glm::mat2x3, GL_FLOAT_MAT2x3, UNIFORM_FUNCTIONS_MATRIX(2, 3))
GetTypeInfoSpecializationDefinition(glm::mat2x4, GL_FLOAT_MAT2x4, UNIFORM_FUNCTIONS_MATRIX(2, 4))

GetTypeInfoSpecializationDefinition(glm::mat3, GL_FLOAT_MAT3, UNIFORM_FUNCTIONS_MATRIX_SQUARE(3))
GetTypeInfoSpecializationDefinition(glm::mat3x2, GL_FLOAT_MAT3x2, UNIFORM_FUNCTIONS_MATRIX(3, 2))
GetTypeInfoSpecializationDefinition(glm::mat3x4, GL_FLOAT_MAT3x4, UNIFORM_FUNCTIONS_MATRIX(3, 4))

GetTypeInfoSpecializationDefinition(glm::mat4, GL_FLOAT_MAT4, UNIFORM_FUNCTIONS_MATRIX_SQUARE(4))
GetTypeInfoSpecializationDefinition(glm::mat4x2, GL_FLOAT_MAT4x2, UNIFORM_FUNCTIONS_MATRIX(4, 2))
GetTypeInfoSpecializationDefinition(glm::mat4x3, GL_FLOAT_MAT4x3, UNIFORM_FUNCTIONS_MATRIX(4, 3))

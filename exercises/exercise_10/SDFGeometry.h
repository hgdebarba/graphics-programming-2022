#pragma once

#include <glad/glad.h>

class SDFGeometry
{
public:

    SDFGeometry(bool isWorld, GLenum primitive, unsigned int vertexCount, const float *vertices, unsigned int indexCount, const GLuint* indices = nullptr, GLsizeiptr stride = 3 * sizeof(float));

    bool GetIsWorld() const { return m_IsWorld; }

    void Draw() const;

private:
    GLenum m_Primitive;
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_EBO;
    GLsizei m_IndexCount;

    bool m_IsWorld;
};

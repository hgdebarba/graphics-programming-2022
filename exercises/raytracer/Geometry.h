#pragma once

#include <glad/glad.h>
#include <initializer_list>

class Geometry
{
public:

    Geometry(GLenum primitive, std::initializer_list<float> vertices, std::initializer_list<unsigned int> indices = {}, GLsizeiptr stride = 3 * sizeof(float));

    void Draw() const;

private:
    GLenum m_Primitive;
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_EBO;
    GLsizei m_DrawCount;
};

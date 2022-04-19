#include "SDFGeometry.h"

SDFGeometry::SDFGeometry(bool isWorld, GLenum primitive, unsigned int vertexCount, const float* vertices, unsigned int indexCount, const GLuint* indices, GLsizeiptr stride)
    : m_IsWorld(isWorld), m_Primitive(primitive), m_VAO(0), m_VBO(0), m_EBO(0), m_IndexCount(indexCount)
{
    // Generate the VAO and bind it
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Generate the VBO, bind it, and initialize the vertex buffer with the vertices data
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * stride, vertices, GL_STATIC_DRAW);

    // With VAO and VBO bound, enable the position attribute and configure it
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, nullptr);

    // optional EBO
    if (indices)
    {
        // Generate the EBO, bind it, and initialize the element buffer with the indices data
        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLuint), indices, GL_STATIC_DRAW);
    }

    // Unbind VAO, VBO and EBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void SDFGeometry::Draw() const
{
    glBindVertexArray(m_VAO);
    if (m_EBO)
    {
        glDrawElements(m_Primitive, m_IndexCount, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
        glDrawArrays(m_Primitive, 0, m_IndexCount);
    }
    glBindVertexArray(0);
}


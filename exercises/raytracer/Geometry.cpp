#include "Geometry.h"

Geometry::Geometry(GLenum primitive, std::initializer_list<float> vertices, std::initializer_list<unsigned int> indices, GLsizeiptr stride)
    : m_Primitive(primitive), m_VAO(0), m_VBO(0), m_EBO(0), m_DrawCount(0)
{
    // Generate the VAO and bind it
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Generate the VBO, bind it, and initialize the vertex buffer with the vertices data
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.begin(), GL_STATIC_DRAW);

    // With VAO and VBO bound, enable the position attribute and configure it
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (GLsizei)stride, nullptr);

    m_DrawCount = (GLsizei)(vertices.size() * sizeof(float) / stride);

    // optional EBO
    if (indices.size() > 0)
    {
        // Generate the EBO, bind it, and initialize the element buffer with the indices data
        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.begin(), GL_STATIC_DRAW);

        m_DrawCount = (GLsizei)indices.size();
    }

    // Unbind VAO, VBO and EBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void Geometry::Draw() const
{
    glBindVertexArray(m_VAO);
    if (m_EBO)
    {
        glDrawElements(m_Primitive, m_DrawCount, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
        glDrawArrays(m_Primitive, 0, m_DrawCount);
    }
    glBindVertexArray(0);
}


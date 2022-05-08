#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
    : m_Position(0.0f, 0.0f, 10.0f), m_LookAt(0.0f), m_Up(0.0f, 1.0f, 0.0f)
    , m_Fov(glm::radians(60.0f)), m_Aspect(1.0f), m_Near(0.1f), m_Far(100.0f)
{
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(m_Position, m_LookAt, m_Up);
}

glm::mat4 Camera::GetProjMatrix() const
{
    return glm::perspective(m_Fov, m_Aspect, m_Near, m_Far);

}

glm::vec3 Camera::ToViewSpace(glm::vec3 xyz, float w) const
{
    glm::vec4 v(xyz.x, xyz.y, xyz.z, w);
    v = GetViewMatrix()* v;
    return glm::vec3(v.x, v.y, v.z);
}

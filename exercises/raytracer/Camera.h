#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class Camera
{
public:

    Camera();

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjMatrix() const;

    glm::vec3 GetPosition() const { return m_Position; }
    void SetPosition(const glm::vec3 &position) { m_Position = position; }
    glm::vec3 GetLookAt() const { return m_LookAt; }
    void SetLookAt(const glm::vec3 &lookAt) { m_LookAt = lookAt; }
    glm::vec3 GetUpVector() const { return m_Up; }
    void SetUpVector(const glm::vec3 &up) { m_Up = up; }

    float GetFov() const { return m_Fov; }
    void SetFov(float fov) { m_Fov = fov; }
    float GetAspect() const { return m_Aspect; }
    void SetAspect(float aspect) { m_Aspect = aspect; }
    float GetNear() const { return m_Near; }
    void SetNear(float near) { m_Near = near; }
    float GetFar() const { return m_Far; }
    void SetFar(float far) { m_Far = far; }

    glm::vec3 ToViewSpace(glm::vec3 xyz, float w) const;

private:

    glm::vec3 m_Position;
    glm::vec3 m_LookAt;
    glm::vec3 m_Up;

    float m_Fov;
    float m_Aspect;
    float m_Near;
    float m_Far;
};


#include "RayTracer.h"

#include <glm/gtc/matrix_transform.hpp>

#include <unordered_set>

#include "Shader.h"

#include <GLFW/glfw3.h>

std::string RayTracer::s_RayTracerSource;
std::string RayTracer::s_LibrarySource;

bool RayTracer::s_SourceChanged(false);

RayTracer::RayTracer(const char* fragmentPath)
    : m_Shader(SHADER_FOLDER "raytracer.vert", fragmentPath)
    , m_Material(&m_Shader)
    , m_FullscreenQuad(GL_TRIANGLE_STRIP, { -1.0f, 1.0f, 0.0f,   -1.0f, -1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   1.0f, -1.0f, 0.0f })
{
}

void RayTracer::Render() const
{
    glm::mat4 viewMatrix = m_Camera.GetViewMatrix();
    glm::mat4 projMatrix = m_Camera.GetProjMatrix();;

    m_Material.Use();

    float currentFrame = (float)glfwGetTime();
    m_Shader.SetUniform("_rt_Time", &currentFrame);

    m_Shader.SetUniform("_rt_View", &viewMatrix);
    m_Shader.SetUniform("_rt_InvView", &inverse(viewMatrix));
    m_Shader.SetUniform("_rt_Proj", &projMatrix);
    m_Shader.SetUniform("_rt_InvProj", &inverse(projMatrix));

    m_FullscreenQuad.Draw();
}

void RayTracer::ReloadShaders()
{
    ReloadSource();

    m_Shader.Reload();

    s_SourceChanged = false;
}

void RayTracer::ReloadSource()
{
    std::size_t beforeRayTracerHash = std::hash<std::string>{}(s_RayTracerSource);
    std::size_t beforeLibraryHash = std::hash<std::string>{}(s_LibrarySource);

    Shader::ReadShaderFile(SHADER_FOLDER "raytracer.glsl", s_RayTracerSource);
    Shader::ReadShaderFile(SHADER_FOLDER "library.glsl", s_LibrarySource);

    std::size_t afterRayTracerHash = std::hash<std::string>{}(s_RayTracerSource);
    std::size_t afterLibraryHash = std::hash<std::string>{}(s_LibrarySource);

    s_SourceChanged = beforeRayTracerHash != afterRayTracerHash || beforeLibraryHash != afterLibraryHash;
}

bool RayTracer::HasSourceChanged()
{
    return s_SourceChanged;
}

const char* RayTracer::GetRayTracerSource()
{
    if (s_RayTracerSource.empty())
    {
        ReloadSource();
    }

    return s_RayTracerSource.c_str();
}

const char* RayTracer::GetLibrarySource()
{
    if (s_LibrarySource.empty())
    {
        ReloadSource();
    }

    return s_LibrarySource.c_str();
}

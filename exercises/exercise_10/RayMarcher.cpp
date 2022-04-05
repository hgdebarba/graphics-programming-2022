#include "RayMarcher.h"

#include <glm/gtc/matrix_transform.hpp>

#include <unordered_set>

#include "SDFCamera.h"
#include "SDFObject.h"
#include "SDFGeometry.h"
#include "SDFMaterial.h"
#include "SDFShader.h"


std::string RayMarcher::s_RayMarcherSource;
std::string RayMarcher::s_SDFLibrarySource;

bool RayMarcher::s_SourceChanged(false);

RayMarcher::RayMarcher()
    : m_Camera(nullptr)
{
}

void RayMarcher::AddObject(const SDFObject* object)
{
    m_Objects.push_back(object);
}

bool RayMarcher::RemoveObject(const SDFObject* object)
{
    bool removed = false;
    auto itEnd = m_Objects.end();
    auto it = std::find(m_Objects.begin(), itEnd, object);
    if (it != itEnd)
    {
        *it = *(itEnd - 1);
        m_Objects.pop_back();
        removed = true;
    }
    return removed;
}

void RayMarcher::Render() const
{
    const SDFMaterial* currentMaterial = nullptr;
    unsigned int modelViewUniformIndex = ~0u;
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;

    for (auto& object : m_Objects)
    {
        const SDFMaterial* prevMaterial = currentMaterial;
        currentMaterial = object->GetMaterial();
        if (currentMaterial)
        {
            if (currentMaterial != prevMaterial)
            {
                currentMaterial->Use();

                if (m_Camera)
                {
                    viewMatrix = m_Camera->GetViewMatrix();
                    projMatrix = m_Camera->GetProjMatrix();
                }
                else
                {
                    viewMatrix = glm::mat4(1);
                    projMatrix = glm::mat4(1);
                }

                const SDFShader* shader = currentMaterial->GetShader();
                shader->SetUniform("_rm_Proj", &projMatrix);

                modelViewUniformIndex = shader->GetUniformIndex("_rm_modelView");
            }

            const SDFGeometry* geometry = object->GetGeometry();
            if (modelViewUniformIndex != ~0u)
            {
                glm::mat4 modelView = geometry->GetIsWorld() ? viewMatrix * object->GetModelMatrix() : glm::inverse(projMatrix);
                currentMaterial->GetShader()->SetUniform(modelViewUniformIndex, &modelView);
            }

            geometry->Draw();
        }
    }
}

void RayMarcher::ReloadShaders()
{
    std::unordered_set<const SDFShader*> shaders;

    ReloadSource();

    for (auto& object : m_Objects)
    {
        shaders.insert(object->GetMaterial()->GetShader());
    }

    for (auto& shader : shaders)
    {
        shader->Reload();
    }

    s_SourceChanged = false;
}

void RayMarcher::ReloadSource()
{
    std::size_t beforeRayMarcherHash = std::hash<std::string>{}(s_RayMarcherSource);
    std::size_t beforeSDFLibraryHash = std::hash<std::string>{}(s_SDFLibrarySource);

    SDFShader::ReadShaderFile(SHADER_FOLDER "raymarcher.glsl", s_RayMarcherSource);
    SDFShader::ReadShaderFile(SHADER_FOLDER "sdflibrary.glsl", s_SDFLibrarySource);

    std::size_t afterRayMarcherHash = std::hash<std::string>{}(s_RayMarcherSource);
    std::size_t afterSDFLibraryHash = std::hash<std::string>{}(s_SDFLibrarySource);

    s_SourceChanged = beforeRayMarcherHash != afterRayMarcherHash || beforeSDFLibraryHash != afterSDFLibraryHash;
}

bool RayMarcher::HasSourceChanged()
{
    return s_SourceChanged;
}

const char* RayMarcher::GetRayMarcherSource()
{
    if (s_RayMarcherSource.empty())
    {
        ReloadSource();
    }

    return s_RayMarcherSource.c_str();
}

const char* RayMarcher::GetSDFLibrarySource()
{
    if (s_SDFLibrarySource.empty())
    {
        ReloadSource();
    }

    return s_SDFLibrarySource.c_str();
}

#pragma once

#include <vector>
#include <string>
#include "Camera.h"
#include "Material.h"
#include "Geometry.h"


class RayTracer
{
public:
    RayTracer(const char* fragmentPath);

    void Render() const;

    void ReloadShaders();

    const Camera& GetCamera() const { return m_Camera; }
    Camera& GetCamera() { return m_Camera; }

    const Material& GetMaterial() const { return m_Material; }
    Material& GetMaterial() { return m_Material; }

    static const char* GetRayTracerSource();
    static const char* GetLibrarySource();

    static bool HasSourceChanged();

private:

    static void ReloadSource();

    Camera m_Camera;
    Shader m_Shader;
    Material m_Material;
    Geometry m_FullscreenQuad;

    static std::string s_RayTracerSource;
    static std::string s_LibrarySource;

    static bool s_SourceChanged;
};

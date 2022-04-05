#pragma once

#include <vector>
#include <string>

class SDFCamera;
class SDFObject;

class RayMarcher
{
public:
    RayMarcher();

    void AddObject(const SDFObject* object);
    bool RemoveObject(const SDFObject* object);

    void Render() const;

    void ReloadShaders();

    const SDFCamera* GetCamera() const { return m_Camera; }
    void SetCamera(const SDFCamera* camera) { m_Camera = camera; };

    static const char* GetRayMarcherSource();
    static const char* GetSDFLibrarySource();

    static bool HasSourceChanged();

private:

    static void ReloadSource();

    const SDFCamera *m_Camera;

    std::vector<const SDFObject*> m_Objects;

    static std::string s_RayMarcherSource;
    static std::string s_SDFLibrarySource;

    static bool s_SourceChanged;
};

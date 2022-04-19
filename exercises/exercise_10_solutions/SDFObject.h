#pragma once

#include <glm/glm.hpp>

class SDFGeometry;
class SDFMaterial;

class SDFObject
{
public:

    SDFObject(const SDFGeometry* geometry, const SDFMaterial* material);

    glm::mat4 GetModelMatrix() const { return m_ModelMatrix; }
    void SetModelMatrix(const glm::mat4 &modelMatrix) { m_ModelMatrix = modelMatrix; }

    const SDFGeometry* GetGeometry() const { return m_Geometry; }
    const SDFMaterial* GetMaterial() const { return m_Material; }

private:
    const SDFGeometry* m_Geometry;
    const SDFMaterial* m_Material;

    glm::mat4 m_ModelMatrix;
};


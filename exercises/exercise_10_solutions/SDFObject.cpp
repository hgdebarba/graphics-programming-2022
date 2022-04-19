#include "SDFObject.h"

#include "SDFGeometry.h"
#include "SDFMaterial.h"

SDFObject::SDFObject(const SDFGeometry* geometry, const SDFMaterial* material)
    : m_ModelMatrix(1.0f), m_Geometry(geometry), m_Material(material)
{
}


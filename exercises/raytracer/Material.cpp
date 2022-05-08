#include "Material.h"

Material::Material(const Shader* shader)
    :m_Shader(shader)
{
}

void Material::Use() const
{
    m_Shader->Use();

    for (auto& it : m_Properties)
    {
        m_Shader->SetUniform(it.second, &m_ValuePool[it.first]);
    }
}

Material::PropertyID Material::FindProperty(const char* name)
{
    PropertyID propertyId = InvalidPropertyID;

    int location = m_Shader->GetUniformLocation(name);
    if (location >= 0)
    {
        auto it = m_LocationProperties.find(location);
        if (it == m_LocationProperties.end())
        {
            propertyId = AddProperty(location);
        }
        else
        {
            propertyId = it->second;
        }
    }
    else
    {
        auto it = m_MissingProperties.find(name);
        if (it == m_MissingProperties.end())
        {
            m_MissingProperties.insert(name);
            std::cout << "Property not found: " << name << std::endl;
        }
    }

    return propertyId;
}

Material::PropertyID Material::AddProperty(int location)
{
    unsigned int uniformIndex = m_Shader->GetUniformIndex(location);

    unsigned int uniformSize = m_Shader->GetUniformSize(uniformIndex);

    unsigned int propertySize = uniformSize / sizeof(int);

    size_t currentPoolSize = m_ValuePool.size();
    PropertyID propertyId = (unsigned int)currentPoolSize;
    m_ValuePool.resize(currentPoolSize + propertySize, 0);

    m_Properties[propertyId] = uniformIndex;
    m_LocationProperties[location] = propertyId;
    return propertyId;
}


#pragma once

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

class SDFShader;

class SDFMaterial
{
public:
    SDFMaterial(const SDFShader* shader);

    typedef unsigned int PropertyID;
    static const PropertyID InvalidPropertyID = ~0u;

    PropertyID FindProperty(const char* name);

    template <typename T>
    bool SetPropertyValue(const char* name, const T& value) { return SetPropertyValue<T>(FindProperty(name), value); }
    template <typename T>
    bool SetPropertyValue(PropertyID propertyId, const T& value);

    template <typename T>
    bool GetPropertyValue(const char* name, T& value) const { return GetPropertyValue<T>(FindProperty(name), value); }
    template <typename T>
    bool GetPropertyValue(PropertyID propertyId, T& value) const;

    template <typename T>
    T* GetPropertyPointer(const char* name) { return GetPropertyPointer<T>(FindProperty(name)); }
    template <typename T>
    T* GetPropertyPointer(PropertyID propertyId);

    const SDFShader* GetShader() const { return m_Shader; }

    void Use() const;

private:

    PropertyID AddProperty(int location);

    template <typename T>
    bool ValidateProperty(PropertyID propertyId) const;

    const SDFShader* m_Shader;

    std::unordered_map<PropertyID, unsigned int> m_Properties;
    std::unordered_map<int, PropertyID> m_LocationProperties;

    std::unordered_set<std::string> m_MissingProperties;

    static_assert(sizeof(int) == sizeof(float), "Using the same pool for ints and floats, size must match");
    std::vector<int> m_ValuePool;
};


template <typename T>
bool SDFMaterial::ValidateProperty(PropertyID propertyId) const
{
    bool valid = false;

    if (propertyId != InvalidPropertyID)
    {
        auto it = m_Properties.find(propertyId);
        if (it != m_Properties.end())
        {
            valid = m_Shader->ValidateUniform<T>(it->second);
        }

        if (!valid)
            std::cout << "Invalid Property: " << propertyId << std::endl;
    }

    return valid;
}


template <typename T>
bool SDFMaterial::SetPropertyValue(PropertyID propertyId, const T& value)
{
    bool valid = ValidateProperty<T>(propertyId);
    if (valid)
    {
        memcpy(&m_ValuePool[propertyId], &value, sizeof(T));
    }
    return valid;
}

template <typename T>
bool SDFMaterial::GetPropertyValue(PropertyID propertyId, T& value) const
{
    bool valid = ValidateProperty<T>(propertyId);
    if (valid)
    {
        memcpy(&value, &m_ValuePool[propertyId], sizeof(T));
    }
    return valid;
}

template <typename T>
T* SDFMaterial::GetPropertyPointer(PropertyID propertyId)
{
    bool valid = ValidateProperty<T>(propertyId);
    return valid ? reinterpret_cast<T*>(&m_ValuePool[propertyId]) : nullptr;
}

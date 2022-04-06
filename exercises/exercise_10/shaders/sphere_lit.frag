
// Uniforms
uniform vec3 sphereColor;

// Constants
const vec3 lightDir = vec3(-1, 1, 1);
const vec3 lightColor = vec3(1, 1, 1);
const float lightIntensity = 1.0f;
const vec3 ambientLightColor = vec3(0.1f, 0.1f, 0.1f);

// Configure ray marcher
void GetRayMarcherConfig(out int maxSteps, out float maxDistance, out float surfaceDistance)
{
    maxSteps = 100;
    maxDistance = 100.0f;
    surfaceDistance = 0.001f;
}

struct Material
{
    vec3 color;
    vec4 phong;
};

struct Output
{
    vec3 normal;
    Material material;
};

// Default value for o
void InitOutput(out Output o)
{
    o.normal = vec3(0, 0, 1);
    o.material.color = sphereColor;
    o.material.phong = vec4(1, 1, 1, 10);
}

// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
    // Get sphere center from modelView matrix position
    vec3 center = _rm_modelView[3].xyz;

    // Get sphere radius from modelView matrix scale
    float scaleX = length(_rm_modelView[0].xyz);
    float scaleY = length(_rm_modelView[1].xyz);
    float scaleZ = length(_rm_modelView[2].xyz);
    float radius = min(scaleX, min(scaleY, scaleZ));

    return sdfSphere(transformToLocal(p, center), o.normal, radius);
}


// Output function: Blinn-Phong lighting
vec4 GetOutputColor(vec3 point, float distance, Output o)
{
    vec3 P = point;
    vec3 N = o.normal;
    vec3 L = normalize(lightDir);
    vec3 V = normalize(-P);
    vec3 H = normalize(L + V);

    vec3 albedo = o.material.color;

    float ambientReflectance = o.material.phong.x;
    vec3 ambient = ambientLightColor * ambientReflectance * albedo;

    float diffuseReflectance = o.material.phong.y;
    vec3 diffuse = diffuseReflectance * albedo;

    float specularReflectance = o.material.phong.z;
    float specularExponent = o.material.phong.w;
    float specModulation = pow(max(dot(H, N), 0.0f), specularExponent);
    vec3 specular = vec3(1.0f) * specularReflectance * specModulation;

    vec3 lighting = ambient + (diffuse + specular) * lightColor * lightIntensity * max(dot(N, L), 0.0f);
    return vec4(lighting, 1);
}

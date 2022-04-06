
// Uniforms
// TODO 10.1 : Replace constants with uniforms with the same name
const vec3 sphereColor = vec3(0, 0, 1);
const vec3 sphereCenter = vec3(-2, 0, -10);
const float sphereRadius = 1.25f;

const vec3 boxColor = vec3(1, 0, 0);
const mat4 boxMatrix = mat4(1,0,0,0, 0,1,0,0, 0,0,1,0, 2,0,-10,1);
const vec3 boxSize = vec3(1, 1, 1);

// Configure ray marcher
void GetRayMarcherConfig(out int maxSteps, out float maxDistance, out float surfaceDistance)
{
    maxSteps = 100;
    maxDistance = 100.0f;
    surfaceDistance = 0.001f;
}

struct Output
{
    // color of the closest figure
    vec3 color;
};

// Default value for o
void InitOutput(out Output o)
{
    o.color = vec3(0.0f);
}

// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
    // Sphere in position "sphereCenter" and size "sphereRadius"
    float dSphere = sdfSphere(transformToLocal(p, sphereCenter), sphereRadius);

    // Box with worldView transform "boxMatrix" and dimensions "boxSize"
    float dBox = sdfBox(transformToLocal(p, boxMatrix), boxSize);

    // TODO 10.1 : Replace min with smin and try different small values of k
    float d = min(dSphere, dBox);

    // TODO 10.1 : Replace this with a mix, using the blend factor from smin
    o.color = d == dSphere ? sphereColor : boxColor;

    return d;
}

// Output function: Just a dot with the normal and view vectors
vec4 GetOutputColor(vec3 p, float distance, Output o)
{
    vec3 normal = calculateNormal(p);
    float dotNV = dot(normalize(-p), normal);
    return vec4(dotNV * o.color, 1.0f);
}

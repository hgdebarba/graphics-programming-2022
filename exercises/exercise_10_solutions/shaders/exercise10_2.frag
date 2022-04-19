
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

uniform float currentTime = 0;

// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
    float power = 1. + (8.-1.)*(0.5 - cos(currentTime*radians(360.)/73.)*0.5);
    vec3 z = p + vec3(0, 0, 1.5);
    float dr = 1.0;
    float r = 0.0;
    for (int i = 0; i < 64 ; i++)
    {
        r = length(z);
        if (r>3.) break;
        
        // convert to polar coordinates
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);

        dr =  pow( r, power-1.0)*power*dr + 1.0;
        
        // scale and rotate the point
        float zr = pow( r,power);
        theta = theta*power;
        phi = phi*power;
        
        // convert back to cartesian coordinates
        z = zr*vec3(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
        z+=p;
    }
    o.color = vec3(0.2f,0.2f,1.0f);
    return 0.5*log(r)*r/dr;
}

// Constants
const vec3 lightDir = vec3(-1, 1, 1);
const vec3 lightColor = vec3(1, 1, 1);
const float lightIntensity = 1.0f;
const vec3 ambientLightColor = vec3(0.1f, 0.1f, 0.1f);
const vec3 reflectance = vec3(1, 1, 1);
const float specularIntensity = 10.0f;

// Output function: Blinn-Phong lighting
vec4 GetOutputColor(vec3 point, float distance, Output o)
{
    vec3 P = point;
    vec3 N = calculateNormal(point);
    vec3 L = normalize(lightDir);
    vec3 V = normalize(-P);
    vec3 H = normalize(L + V);

    vec3 albedo = o.color;

    float ambientReflectance = reflectance.x;
    vec3 ambient = ambientLightColor * ambientReflectance * albedo;

    float diffuseReflectance = reflectance.y;
    vec3 diffuse = diffuseReflectance * albedo;

    float specularReflectance = reflectance.z;
    float specularExponent = specularIntensity;
    float specModulation = pow(max(dot(H, N), 0.0f), specularExponent);
    vec3 specular = vec3(1.0f) * specularReflectance * specModulation;

    vec3 lighting = ambient + (diffuse + specular) * lightColor * lightIntensity * max(dot(N, L), 0.0f);
    return vec4(lighting, 1);
}


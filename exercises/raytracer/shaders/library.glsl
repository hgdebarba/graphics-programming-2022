#version 330 core

// No main in this file, it is just a library

uniform mat4 _rt_View;
uniform mat4 _rt_InvView;
uniform mat4 _rt_Proj;
uniform mat4 _rt_InvProj;

uniform float _rt_Time;

struct Ray
{
    vec3 point;
    vec3 direction;
    vec3 colorFilter;
};

struct Material
{
    vec3 color;
};

struct Output
{
    vec3 point;
    vec3 normal;
    vec3 refractPoint;
    vec3 refractDirection;
    Material material;
};

// Fill in this function to define your scene
bool castRay(Ray ray, inout float distance, out Output o);
bool castRay(Ray ray, inout float distance)
{
    Output o;
    return castRay(ray, distance, o);
}

// Fill in this function to process the output once the ray has found a hit
vec3 ProcessOutput(Ray ray, Output o);

// Function to enable recursive rays
bool PushRay(vec3 point, vec3 direction, vec3 colorFilter);



struct Sphere
{
    vec3 center;
    float radius;
    Material material;
};

bool raySphereIntersection(Ray ray, Sphere sphere, inout float distance, inout Output o)
{
    bool hit = false;

    vec3 m = ray.point - sphere.center;

    float b = dot(m, ray.direction); 
    float c = dot(m, m) - sphere.radius * sphere.radius; 

    if (c <= 0.0f || b <= 0.0f)
    {
        float discr = b * b - c;
        if (discr >= 0.0f)
        {
            float d = max(-b - sqrt(discr), 0.0f);

            if (d < distance)
            {
                distance = d;

                o.point = ray.point + d * ray.direction;
                o.normal = normalize(o.point - sphere.center);
                o.material = sphere.material;

                hit = true;
            }
        }
    }

    return hit;
}


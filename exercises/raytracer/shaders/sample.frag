
const uint _rm_MaxRays = 100u;

bool castRay(Ray ray, inout float distance, out Output o)
{
    Sphere sphere;
    sphere.center = vec3(0,0,-10);
    sphere.radius = 2.0f;
    sphere.material.color = vec3(1);

    bool hit = false;
    for (int i = 1; i <= 10; ++i)
    {
        vec3 offset = 5.0f * vec3(sin(3*i+_rt_Time), sin(2*i+_rt_Time), sin(4*i+_rt_Time));
        sphere.center = offset + vec3(0,0,-20);
        sphere.material.color = normalize(offset) * 0.5f + 0.5f;
        hit = raySphereIntersection(ray, sphere, distance, o) || hit;
    }

    return hit;
}

vec3 ProcessOutput(Ray ray, Output o)
{
    return o.material.color;
}


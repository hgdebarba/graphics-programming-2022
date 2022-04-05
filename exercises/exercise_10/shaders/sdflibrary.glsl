#version 330 core

// No main in this file, it is just a library

// Transforms of the geometry used to render the scene
uniform mat4 _rm_modelView;
uniform mat4 _rm_Proj;





// Transform point relative to a specific position
vec3 transformToLocal(vec3 p, vec3 position)
{
    return p - position;
}

// Transform point relative to a transform matrix
vec3 transformToLocal(vec3 p, mat4 m)
{
    return (inverse(m) * vec4(p, 1)).xyz;
}

// Transform vector relative to a transform matrix
vec3 transformToLocalVector(vec3 v, mat4 m)
{
    return (inverse(m) * vec4(v, 0)).xyz;
}



// Smooth minimum with smoothness k
float smin(float a, float b, float k)
{
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * (1.0 / 4.0);
}

// Smooth minimum with smoothness k and returning blend value in range (0-1)
float smin(float a, float b, float k, out float blend)
{
    vec3 v = max(vec3(k) - vec3(b, a, abs(a - b)), 0.0f) / k;
    blend = (v.x*v.x - v.y*v.y);

    //blend = blend * blend * blend;
    blend = blend * 0.5f + 0.5f;
    float h = v.z;
    return min(a, b) - h * h * k * (1.0 / 4.0);
}



// Calculate numerical normals using the tetrahedron technique with specific differential
vec3 calculateNormal(vec3 p, float h);

// Calculate numerical normals using the tetrahedron technique
vec3 calculateNormal(vec3 p)
{
    return calculateNormal(p, 0.0001f);
}



// Signed distance field of a sphere
float sdfSphere(vec3 p, float radius)
{
    return length(p) - radius;
}

// Signed distance field of a sphere with analytic normal
float sdfSphere(vec3 p, out vec3 normal, float radius)
{
    float distance = length(p);
    normal = p / distance;
    return distance - radius;
}

// Signed distance field of a box
float sdfBox(vec3 p, vec3 halfsize)
{
    vec3 q = abs(p) - halfsize;
    float outerDist = length(max(q, 0.0f));
    float innerDist = min(max(q.x, max(q.y, q.z)), 0.0f);
    return outerDist + innerDist;
}

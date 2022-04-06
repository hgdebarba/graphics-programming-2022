
// version not included here because it is a partial file
//#version 330 core

uniform vec2 _rm_depthRemap;

in vec3 _rm_viewPos;

out vec4 FragColor;


uniform int _rm_hack;
// Calculate numerical normals using the tetrahedron technique with specific differential
// Implementation here because GetDistance needs to be defined
vec3 calculateNormal(vec3 p, float h)
{
    vec3 normal = vec3(0.0f);

    Output o;
    #define ZERO (min(_rm_hack, 0)) // hack to prevent inlining
    for(int i = ZERO; i < 4; i++)
    {
        vec3 e = 0.5773*(2.0*vec3((((i+3)>>1)&1),((i>>1)&1),(i&1))-1.0);
        normal += e * GetDistance(p + e * h, o);
    }

    return normalize(normal);
}

// Ray marching algorithm
float rayMarch(vec3 origin, vec3 dir)
{
    Output o; // intentionally unused, to help compiler optimize
    float distance = 0.0f;

    // Get configuration specific to this shader pass
    int maxSteps;
    float maxDistance, surfaceDistance;
    GetRayMarcherConfig(maxSteps, maxDistance, surfaceDistance);

    // Iterate until maxSteps is reached or we find a point
    for(int i = 0; i < maxSteps; ++i)
    {
        // Get distance to the current point
        vec3 p = origin + dir * distance;
        float d = GetDistance(p, o);
        distance += d;

        // If distance is too big, discard the fragment
        if (distance > maxDistance)
            discard;

        // If this step increment was very small, we found a hit
        if (d < surfaceDistance)
            break;
    }

    return distance;
}

void main()
{
    // Start from transformed position
    vec3 origin = _rm_viewPos;

    // Initial distance to camera
    float distance = length(origin);

    // Normalize to get view direction
    vec3 dir = origin / distance;

    // Get Distance from the origin to the closest object
    distance += rayMarch(origin, dir);

    // Hit point in view space is given by the direction from the camera and the distance
    vec3 point = dir * distance;

    // Invoke GetDistance again to get the o value
    Output o;
    InitOutput(o);
    GetDistance(point, o);

    // With the o value, get the final color
    FragColor = GetOutputColor(point, distance, o);

    // Convert linear depth to normalized depth (same as projecting the point and taking the Z/W)
    gl_FragDepth = -_rm_Proj[2][2] - _rm_Proj[3][2] / point.z;
}

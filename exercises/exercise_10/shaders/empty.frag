
// Configure ray marcher
void GetRayMarcherConfig(out int maxSteps, out float maxDistance, out float surfaceDistance)
{
    maxSteps = 100;
    maxDistance = 100.0f;
    surfaceDistance = 0.001f;
}

// Required struct for o data (can't be empty)
struct Output
{
    float empty;
};

// Default value for o
void InitOutput(out Output o)
{
}

// Signed distance function
float GetDistance(vec3 p, inout Output o)
{
    return 0.0f;
}

// Output function: Compute the final color
vec4 GetOutputColor(vec3 point, float distance, Output o)
{
    return vec4(0.0f, 0.0f, 0.5f, 1.0f);
}

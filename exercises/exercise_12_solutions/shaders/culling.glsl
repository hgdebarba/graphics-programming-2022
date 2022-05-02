#version 430 core

layout(local_size_x = 64) in;

struct InstanceData
{
   mat4 model;
   vec4 color;
};

layout(std430, binding = 0) buffer sourceInstanceData
{
   InstanceData instances[];
};

layout(std430, binding = 1) buffer visibleInstanceData
{
   InstanceData visibles[];
};

layout(std430, binding = 2) buffer indirectData
{
    uint count;
    uint instanceCount;
    uint firstIndex;
    int  baseVertex;
    uint baseInstance;
};

uniform float cullingRadius;
uniform vec3 frustumPlanes[12];

void main()
{
    if (gl_GlobalInvocationID.x < instances.length())
    {
        vec3 center = instances[gl_GlobalInvocationID.x].model[3].xyz;

        bool isVisible = true;
        for(int i = 0; i < 6; ++i)
        {
            vec3 planePoint = frustumPlanes[i * 2];
            vec3 planeNormal = frustumPlanes[i * 2 + 1];
            bool visiblePlane = dot(center - planePoint, planeNormal) > -cullingRadius;
            isVisible = isVisible && visiblePlane;
            if (!isVisible)
                break;
        }

        if (isVisible)
        {
            uint index = atomicAdd(instanceCount, 1);
            visibles[index] = instances[gl_GlobalInvocationID.x];
        }
    }
}
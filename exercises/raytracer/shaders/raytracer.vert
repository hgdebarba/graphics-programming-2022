#version 330 core
layout (location = 0) in vec3 vertex;

uniform mat4 _rt_InvProj;

out vec3 _rt_viewPos;

void main()
{
    vec4 viewPos = _rt_InvProj * vec4(vertex, 1.0f);
    _rt_viewPos = viewPos.xyz / viewPos.w;
    gl_Position = vec4(vertex, 1.0f);
}

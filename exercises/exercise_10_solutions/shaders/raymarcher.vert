#version 330 core
layout (location = 0) in vec3 vertex;

uniform mat4 _rm_modelView;
uniform mat4 _rm_Proj;

out vec3 _rm_viewPos;

void main()
{
    vec4 viewPos = _rm_modelView * vec4(vertex, 1.0f);
    _rm_viewPos = viewPos.xyz / viewPos.w;
    gl_Position = _rm_Proj * viewPos;
}

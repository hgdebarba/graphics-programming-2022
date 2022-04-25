#version 430 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textCoord;
layout (location = 3) in vec3 tangent;

uniform mat4 model; // represents model coordinates in the world coord space
uniform mat4 viewProjection;  // represents the view and projection matrices combined

uniform vec4 reflectionColor;


out vec4 worldPos;
out vec3 worldNormal;
out vec3 worldTangent;
out vec2 textureCoordinates;
out vec4 vertexColor;


struct InstanceData
{
   mat4 model;
   vec4 color;
};

layout(std430, binding = 0) buffer instanceData
{
   InstanceData instances[];
};


void main() {
   // vertex in world space (for lighting computation)
   worldPos = model * vec4(vertex, 1.0);

   // object color
   vertexColor = reflectionColor;

   // if there is a buffer, use it to find the model matrix and the color for this instance
   if (instances.length() > 0)
   {
      worldPos = instances[gl_InstanceID].model * vec4(vertex, 1.0);
      vertexColor = instances[gl_InstanceID].color;
   }

   // normal in world space (for lighting computation)
   worldNormal = (model * vec4(normal, 0.0)).xyz;
   // tangent in world space (for lighting computation)
   worldTangent = (model * vec4(tangent, 0.0)).xyz;

   textureCoordinates = textCoord;

   // final vertex position (for opengl rendering, not for lighting)
   gl_Position = viewProjection * worldPos;
}

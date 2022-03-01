#version 330 core

uniform vec3 camPosition; // so we can compute the view vector
out vec4 FragColor; // the output color of this fragment

// TODO exercise 5.4 setup the 'uniform' variables needed for lighting
// light uniforms

// material uniforms

// TODO exercise 5.4 add the 'in' variables to receive the interpolated Position and Normal from the vertex shader


void main()
{

   // TODO exercise 5.4 - phong shading (i.e. Phong reflection model computed in the fragment shader)
   // ambient component

   // diffuse component for light 1

   // specular component for light 1

   // TODO exercuse 5.5 - attenuation - light 1


   // TODO exercise 5.6 - multiple lights, compute diffuse, specular and attenuation of light 2


   // TODO compute the final shaded color (e.g. add contribution of the attenuated lights 1 and 2)


   // TODO set the output color to the shaded color that you have computed
   FragColor = vec4(.8, .8, .8, 1.0);
}

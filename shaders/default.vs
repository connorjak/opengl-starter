#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float scale;
uniform vec3 model_center;
out vec3 vertColor;
out vec3 normal;
void main()
{
   gl_Position = projection * view * model * vec4(aPos*scale, 1.0);
   vertColor = aColor;
   normal = normalize(aPos-model_center);
}
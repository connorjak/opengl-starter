#version 330 core
uniform vec3 lightDir;
out vec4 FragColor;
in vec3 vertColor;
in vec3 normal;
void main()
{
   FragColor = vec4(vertColor, 1.0f) * dot(normal, -lightDir);
}
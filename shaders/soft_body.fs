#version 330 core
uniform vec3 lightDir;
uniform float age;
out vec4 FragColor;
in vec3 vertColor;
in vec3 normal;
void main()
{
   vec3 lightColor = vec3(1,1,1);
   float ambientStrength = 0.1;
   vec3 ambientLight = ambientStrength * lightColor;

   vec3 ambient = ambientLight * vertColor;
   vec3 baseColor = normal;
   FragColor = vec4(baseColor, 1.0f);
}
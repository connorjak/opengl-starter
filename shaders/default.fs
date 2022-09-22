#version 330 core
uniform vec3 lightDir;
uniform float age;
out vec4 FragColor;
in vec3 vertColor;
in vec3 normal;
void main()
{
   vec3 orange = vec3(246.0/255.0, 120.0/255.0, 29.0/255.0);
   vec3 baseColor = mix(vertColor, orange, clamp(age/20.0, 0, 1));
   FragColor = vec4(baseColor, 1.0f) * dot(normal, -lightDir);
}
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
   vec3 orange = vec3(246.0/255.0, 120.0/255.0, 29.0/255.0);
   vec3 blue = vec3(2/255.0, 154.0/255.0, 228.0/255.0);
   vec3 baseColor = vertColor * mix(orange, blue, clamp(age/20.0, 0, 1));
   // FragColor = vec4(baseColor, 1.0f) * (dot(normal, -lightDir) + ambient);
   // FragColor = vec4(baseColor, 1.0f) * dot(normal, -lightDir);
   FragColor = vec4(baseColor, 1.0f);
}
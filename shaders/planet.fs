#version 330 core
uniform vec3 lightDir;
out vec4 FragColor;
in vec3 vertColor;
in vec3 normal;
void main()
{
	vec3 lightColor = vec3(1,1,1);
	float ambientStrength = 0.1;
    vec3 ambientLight = ambientStrength * lightColor;

    vec3 ambient = ambientLight * vertColor;
	// FragColor = vec4(vertColor, 1.0f) * dot(normal, -lightDir) + vec4(ambient, 1.0f);
	FragColor = vec4(vertColor, 1.0f) * dot(normal, -lightDir);
}
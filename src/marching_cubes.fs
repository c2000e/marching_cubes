#version 330 core

// Calculates Phong lighting based on passed-in vertex and normal data.

in vec3 position;
in vec3 normal;

out vec4 color;

uniform vec3 viewPos;

void main()
{
	vec3 viewDir = normalize(viewPos - position);

	vec3 lightPos = vec3(15.0, 10.0, 10.0);
	vec3 lightDir = normalize(lightPos - position);
	vec3 lightCol = vec3(1.0, 1.0, 1.0);

	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightCol;

	float diffuseStrength = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diffuseStrength * lightCol;

	float specularStrength = 0.5;
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightCol;

	vec3 result = (ambient + diffuse + specular) * normal;
	color = vec4(result, 1.0);
}
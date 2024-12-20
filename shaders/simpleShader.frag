#version 330 core
out vec4 color;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
	// Ambient lighting
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse lighting
	float diffuseStrength = 0.5f;
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = (diffuseStrength * diff) * lightColor;

	// Specular lighting
	float specularStrength = 0.3f;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	vec3 specular = (specularStrength * spec) * lightColor;

	// Total lighting model
	vec3 result = (ambient + diffuse + specular) * objectColor;
	color = vec4(result, 1.0f);
}

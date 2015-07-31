#version 330 core
struct Material {
	sampler2D diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

out vec4 color;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform Light light;
uniform Material material;
uniform vec3 viewPos;

void main()
{
	// Ambient lighting
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));

	// Diffuse lighting
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

	// Specular lighting
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = spec * material.specular;

	// Total lighting model
	color = vec4(ambient + diffuse + specular, 1.0f);
}

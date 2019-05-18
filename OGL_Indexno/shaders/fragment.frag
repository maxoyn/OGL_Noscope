#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1; //only diffuse map, the ambient usually has the same color as the diffuse 
    sampler2D texture_specular1;    
    float shininess;
}; 

struct Light {
    vec3 position;
    vec3 diffuse; //the color of the light
	vec3 direction;
	float cutOff;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
  
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

vec4 calcFlash(vec3 res) {
	vec3 lightDir = normalize(light.position - FragPos);
	float theta = dot(lightDir, normalize(-light.direction));
	if(theta>light.cutOff) {
	FragColor = vec4(res, 1.0);
	}
	else
	res -= light.diffuse;
	FragColor = vec4(res, 1.0);

	return FragColor;
}

void main()
{
    // ambient
    vec3 ambient = texture(material.texture_diffuse1, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); //cos to light direction
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
	//cos to the power of shininess of angle between light reflected ray and view direction
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess); 
    vec3 specular = spec * texture(material.texture_specular1, TexCoords).rgb;  
      
    vec3 result = ambient + diffuse + specular;
    //FragColor = vec4(specular, 1.0);

	//FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	
	vec4 finalcolor = calcFlash(result);
	FragColor = vec4(result, 1.0) * finalcolor;
} 



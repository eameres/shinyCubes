#version 330 core

out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

in vec3 Normal;  
in vec3 FragPos;  
in vec3 viewPos;

vec3 lightPos = vec3(0.0,0.0,8.0);
vec3 lightColor = vec3(1,1,1);
vec3 objColor;
float specularStrength = 1.5;

uniform sampler2D ourTexture;
uniform samplerCube env;

void main()
{
	objColor =  vec3(texture(ourTexture,TexCoord)) * .5;

	vec3 r = reflect(normalize(FragPos-viewPos), normalize(Normal));
	vec4 envColor = texture(env,r);

    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm); 

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = specularStrength * spec * lightColor;  
            
    vec3 result = (ambient + diffuse + specular) * objColor;
    FragColor = vec4(result, 1.0) + (envColor * .2);
}



















#version 330 core
out vec4 FragColor;

struct Material {
    // sampler2D diffuse;
    vec3      specular;    
    float     shininess;    
};

struct LightAtten {
    float constant;
    float linear;
    float quadratic;  
};

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;    
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutoff;
    float outerCutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
};

#define NR_POINT_LIGHTS 4

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
  
uniform vec3 viewPos;
uniform Material material;

uniform LightAtten lightAtten;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

uniform float mixValue;
uniform sampler2D texture1;
uniform sampler2D texture2;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texture);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture);

void main() {
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 texture = mix(texture(texture1, TexCoord).rgb, 
                    texture(texture2, TexCoord).rgb, 
                    mixValue);

    vec3 result  = CalcDirLight(dirLight, norm, viewDir, texture);

    for(int i = 0; i < NR_POINT_LIGHTS; i++) {
        result  += CalcPointLight(pointLights[i], norm, FragPos, viewDir, texture);
    }

    result      += CalcSpotLight(spotLight, norm, FragPos, viewDir, texture);

    FragColor    = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 texture) {
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * texture;
    vec3 diffuse = light.diffuse * diff * texture;
    vec3 specular = light.specular * spec * material.specular;
    return (ambient + diffuse + specular);

}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture) {
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float lightDistance = length(light.position - fragPos);
    float attenuation = 1.0 / ( lightAtten.constant  + 
                                lightAtten.linear    *   lightDistance + 
                                lightAtten.quadratic * ( lightDistance * lightDistance ) );    
    // combine results
    vec3 ambient = light.ambient * texture;
    vec3 diffuse = light.diffuse * diff * texture;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 texture) {
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float lightDistance = length(light.position - fragPos);
    float attenuation = 1.0 / ( lightAtten.constant  + 
                                lightAtten.linear    *   lightDistance + 
                                lightAtten.quadratic * ( lightDistance * lightDistance ) );
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * texture;
    vec3 diffuse = light.diffuse * diff * texture;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    vec3  specular;    
    float shininess;    
};

struct LightAtten {
    float constant;
    float linear;
    float quadratic;  
};

struct Light {
    // point light attr & spot light attr 
    vec3 position;
    // directional light attr
    vec3 direction;

    // spot light attr
    float cutoff;
    float outerCutoff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;  
};

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
  
uniform vec3 viewPos;
uniform Material material;

// uniform Light directionalLight;
uniform Light pointLight;
uniform Light spotLight;
uniform LightAtten lightAtten;

uniform float mixValue;
uniform sampler2D texture1;
uniform sampler2D texture2;

vec3 lightShader(Light light, vec3 texture, bool isSpotLight) {
    // ambient
    vec3 ambient = light.ambient * texture;

    // diffuse
    vec3 norm     = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = light.diffuse * (diff * texture);

    // specular
    vec3 viewDir    = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec      = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular   = light.specular * (spec * material.specular);

    // spot light (soft edges)
    if(isSpotLight) {
        float theta     = dot(lightDir, normalize(-light.direction));
        float epsilon   = light.cutoff - light.outerCutoff;
        float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
        diffuse  *= intensity;
        specular *= intensity;
    }
    
    // attenuation
    float lightDistance = length(light.position - FragPos);
    float attenuation   = 1.0 / (lightAtten.constant  + 
                                 lightAtten.linear    *  lightDistance + 
                                 lightAtten.quadratic * (lightDistance * lightDistance));

    return attenuation * (ambient + diffuse + specular);
}

void main()
{
    // texture
    vec3 texture = mix(texture(texture1, TexCoord).rgb, 
                       texture(texture2, TexCoord).rgb, 
                       mixValue);

    // vec3 pointResult = lightShader(pointLight, texture, false);
    vec3 spotResult = lightShader(spotLight, texture, true);

    // FragColor = vec4(pointResult + spotResult, 1.0);
    FragColor = vec4(spotResult, 1.0);
}
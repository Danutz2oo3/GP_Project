#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fragTexCoords; // Add this line

out vec4 fColor;

// lighting
uniform vec3 lightPos;
uniform vec3 lightColor;

// texture samplers
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;
float constant = 1.0f;
float linear = 0.0045f;    // You may need to adjust these values for your scene
float quadratic = 0.0075f; // You may need to adjust these values for your scene

void computeLightComponents()
{
    vec3 cameraPosEye = vec3(0.0f); // in eye coordinates, the viewer is situated at the origin
    
    // transform normal
    vec3 normalEye = normalize(fNormal);    
    
    // compute light direction
    vec3 lightDir = normalize(lightPos - fPosEye.xyz);
    
    // compute view direction 
    vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

    // compute distance to the light source
    float dist = length(lightPos - fPosEye.xyz);
    float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));
        
    // compute ambient light
    ambient = att * ambientStrength * lightColor;
    
    // compute diffuse light
    diffuse = att * max(dot(normalEye, lightDir), 0.0f) * lightColor;
    
    // compute specular light
    vec3 reflection = reflect(-lightDir, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = att * specularStrength * specCoeff * lightColor;
}

void main() 
{
    computeLightComponents();
    
    vec3 baseColor = texture(diffuseTexture, fragTexCoords).rgb;
    
    ambient *= baseColor;
    diffuse *= baseColor;
    specular *= texture(specularTexture, fragTexCoords).rgb;
    
    vec3 color = min((ambient + diffuse) + specular, 1.0f);
    
    fColor = vec4(color, 1.0f);
}
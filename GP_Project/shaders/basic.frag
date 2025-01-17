#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec3 fPosition;
out vec4 fColor;

// Sunlight (Directional Light)
uniform vec3 sunLightDir;
uniform vec3 sunLightColor;
uniform vec3 sunLightPosition;  
uniform mat4 model;  

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};

#define NUM_POINT_LIGHTS 3
uniform PointLight pointLights[NUM_POINT_LIGHTS];

// Camera Position (for specular reflection)
uniform vec3 cameraPos;

// Textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

// Light intensity settings
float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float shininess = 32.0f;

// Attenuation factors for point light
float constant = 1.0f;
float linear = 0.09f;    
float quadratic = 0.032f;

// Lighting components
vec3 ambient;
vec3 diffuse;
vec3 specular;

// Compute point light contribution
void computePointLight(vec3 fragPosWorld, vec3 normalWorld, vec3 viewDir) {
    for (int i = 0; i < NUM_POINT_LIGHTS; i++) {
        vec3 lightDir = normalize(pointLights[i].position - fragPosWorld);

        // **Force the light to shine mainly downward**
        lightDir = normalize(lightDir + vec3(0.0, -1.2, 0.0)); 

        float distance = length(pointLights[i].position - fragPosWorld);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance + pointLights[i].quadratic * (distance * distance));

        // **Increase brightness for better visibility**
        float brightness = max(dot(normalWorld, lightDir), 0.0) * 2.5;  
        
        ambient += pointLights[i].color * 0.2 * attenuation;
        diffuse += pointLights[i].color * brightness * attenuation;
        
        vec3 reflectDir = reflect(-lightDir, normalWorld);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        specular += pointLights[i].color * spec * specularStrength * attenuation;
    }
}


// Compute sunlight (directional light) contribution
void computeSunLight() {		
    vec3 normalEye = normalize(fNormal);	
    vec3 lightDirN = normalize(sunLightDir);
    vec3 viewDirN = normalize(cameraPos - fPosEye.xyz);

    // Compute ambient, diffuse, and specular lighting for sunlight
    ambient = ambientStrength * sunLightColor;
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * sunLightColor;
    
    vec3 reflection = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
    specular = specularStrength * specCoeff * sunLightColor;
}

// Compute shadow mapping
float computeShadow() {
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    normalizedCoords = normalizedCoords * 0.5 + 0.5;
    if (normalizedCoords.z > 1.0f) return 0.0f;

    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
    float currentDepth = normalizedCoords.z;
    float bias = 0.005f;
    float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;
    return shadow;
}

// Main fragment shader logic
void main() {
    computeSunLight();
    
    vec3 fragPosWorld = vec3(model * vec4(fPosition, 1.0));
    vec3 normalWorld = normalize(fNormal);
    vec3 viewDir = normalize(-fragPosWorld);

    computePointLight(fragPosWorld, normalWorld, viewDir); // Apply point lights
    // Apply textures to light components
    vec3 baseColor = texture(diffuseTexture, fTexCoords).rgb;
    ambient *= baseColor;
    diffuse *= baseColor;
    specular *= texture(specularTexture, fTexCoords).rgb;

    // Apply shadowing effect
    float shadow = computeShadow();
    vec3 finalColor = min((ambient + diffuse * (1.0 - shadow) + specular * (1.0 - shadow)), vec3(1.0f));

    fColor = vec4(finalColor, 1.0f);
}

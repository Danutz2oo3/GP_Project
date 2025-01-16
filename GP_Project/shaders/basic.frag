#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fragTexCoords;

out vec4 fColor;

// Time-based lighting (0.0 = night, 1.0 = day)
uniform float timeOfDay;

// Sunlight
uniform vec3 sunDir;
uniform vec3 sunColor;

// Moonlight
uniform vec3 moonDir;
uniform vec3 moonColor;

// Camera position (for reflections)
uniform vec3 cameraPos;

// Texture samplers
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

// Fresnel reflection parameters
float fresnelStrength = 0.04;  // Fresnel base reflection (0.04 for dielectrics)
float roughness = 0.3;         // Surface roughness (adjust for materials)
float metallic = 0.1;          // Determines how metallic the material appears

// Lighting properties
vec3 ambient;
vec3 diffuse;
vec3 specular;

void computeLightComponents() {
    vec3 normalEye = normalize(fNormal);
    vec3 viewDir = normalize(cameraPos - fPosEye.xyz);

    // Interpolate between moonlight and sunlight
    vec3 lightDir = normalize(mix(moonDir, sunDir, timeOfDay));
    vec3 lightColor = mix(moonColor, sunColor, timeOfDay);

    // Compute ambient light
    float ambientStrength = mix(0.1f, 0.4f, timeOfDay);  // Stronger at night
    ambient = ambientStrength * lightColor;

    // Compute diffuse shading
    float diff = max(dot(normalEye, lightDir), 0.0);
    diffuse = diff * lightColor;

    // Compute Fresnel reflection using Schlick's approximation
    float fresnel = fresnelStrength + (1.0 - fresnelStrength) * pow(1.0 - max(dot(viewDir, normalEye), 0.0), 5.0);

    // Compute microfacet-based specular highlights (Cook-Torrance model)
    float specularStrength = mix(0.1f, 0.5f, timeOfDay); // Less at night
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalEye, halfVector), 0.0), (1.0 - roughness) * 128.0);
    specular = specularStrength * spec * lightColor * fresnel;
}

void main() {
    computeLightComponents();
    
    // Sample base texture color
    vec3 baseColor = texture(diffuseTexture, fragTexCoords).rgb;

    // Apply textures to lighting components
    ambient *= baseColor;
    diffuse *= baseColor;
    specular *= texture(specularTexture, fragTexCoords).rgb * metallic;

    // Combine lighting and clamp
    vec3 color = ambient + diffuse + specular;
    color = min(color, vec3(1.0));

    // Apply final color with realistic shading
    fColor = vec4(color, 1.0);
}

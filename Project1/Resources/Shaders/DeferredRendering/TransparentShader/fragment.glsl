#version 460 core
precision highp float;
layout (location = 0) out vec4 FragColour;
layout (location = 1) out vec4 BrightColor; 


// constants
const float PI = 3.14159265359;
const int maxPointLights = 100;
const int maxDirectionalLights = 5;
const int maxSpotLights = 100;

void SpecularTerm(float dotNH, float a2, float cosTheta, vec3 F0, float dotNV, 
                float dotNL, float Roughness, inout vec3 info[2]);
/*vec3 GammaCorrect(vec3 col);
vec3 ToneMap(vec3 a);*/
void ProcessOutputs(vec3 colour, float alphaValue);


// PBR Functions
float Distribution(float dotNH, float a2);
vec3 Reflectance(float cosTheta, vec3 F0);
float Geometry(float dotNV, float dotNL, float Roughness);
float CorrectionFactor(float dotNV, float dotNL);


//Material Data Structures
struct MatItem4 {
    vec4 raw;
    sampler2D id;
    float mixValue;
};
struct MatItem3 {
    vec3 raw;
    sampler2D id;
    float mixValue;
};
struct MatItem1 {
    float raw;
    sampler2D id;
    float mixValue;
};

struct Material {
    MatItem4 albedo;

    MatItem3 normal;

    MatItem1 metalic;

    MatItem1 roughness;

    MatItem1 ao;
};

// Material Functions
vec3 getData(MatItem4 item);
vec3 getData(MatItem3 item);
vec3 getNormal(MatItem3 item, vec3 normal);
float getData(MatItem1 item);
float getAlpha(MatItem4 item);

// Light Source Data Structures
struct PointLight {
    vec3 position;
    vec3 colour;
    float brightness;
};
uniform int numberOfPointLights;
uniform PointLight pointLights[maxPointLights];

struct DirectionalLight {
    vec3 direction;
    vec3 colour;
    float brightness;
};
uniform int numberOfDirectionalLights;
uniform DirectionalLight directionalLights[maxDirectionalLights];

// Light Source Functions
// POINT LIGHTS
vec3 ProcessPointLight(PointLight light, vec3 WFP, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV);
vec3 PointLights(PointLight pointLights[maxPointLights], int numberOfPointLights, vec3 WFP, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV);
// DIRECTIONAL LIGHTS
vec3 ProcessDirectionalLight(DirectionalLight light, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV);
vec3 DirectionalLights(DirectionalLight directionalLights[maxDirectionalLights], int numberOfDirectionalLights, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV);
// SPOT LIGHTS

// Shadows
float CalculateShadowValue(vec4 LSFragmentPosition, vec3 Normal);
uniform sampler2D ShadowMap;
uniform mat4 LSMatrix;
uniform vec3 ShadowCasterPosition;

uniform Material material;

in float CameraExposure;
in vec3 WorldFragmentPosition;
in vec3 CameraPosition;
in vec3 NormalIn;
in vec2 TextureCoords;
in float GammaValue;

void main() {
    // vec3  = texture(positionTex, TextureCoords).xyz;
    vec3 Albedo     = pow(getData(material.albedo), vec3(2.2)); // PBR gamma corrects
    vec3 Normal     = normalize(getNormal(material.normal, NormalIn));
    Normal = normalize(NormalIn);

    float Metallic  = getData(material.metalic);
    Metallic = 0.5;
    float Roughness = getData(material.roughness);
    float AO        = getData(material.ao);

    // shadows
    vec4 LSFragmentPosition    = LSMatrix * vec4(WorldFragmentPosition, 1.0);

    // constants
    vec3 viewDirection = normalize(CameraPosition - WorldFragmentPosition);
    float dotNV = max(dot(Normal, viewDirection), 0.0);
    float alpha = Roughness * Roughness;
    float alpha2 = max(0.001, alpha * alpha);
    float alphaValue = getAlpha(material.albedo);
    vec3 albedoDiffuse = (Albedo * alphaValue) / PI;
    
    // surface reflection at when looking from 0 degrees
    vec3 realSpecular = mix(vec3(0.04), Albedo, Metallic); // 0.04 assumption for metals

    // POINT LIGHTS
    vec3 accumlativeLight = PointLights(pointLights, numberOfPointLights, WorldFragmentPosition, viewDirection, Normal, realSpecular, albedoDiffuse, alpha2, Roughness, Metallic, dotNV);
    // DIRECTIONAL LIGHTS
    accumlativeLight += DirectionalLights(directionalLights, numberOfDirectionalLights, viewDirection, Normal, realSpecular, albedoDiffuse, alpha2, Roughness, Metallic, dotNV);

    float shadow = (1.0 - CalculateShadowValue(LSFragmentPosition, Normal)) * (1.0 - alphaValue);
    shadow = 1;
    vec3 ambient = vec3(0.03) * Albedo * AO;
    vec3 colour = ambient + shadow * accumlativeLight;


    /*colour = ToneMap(colour);
    colour = GammaCorrect(colour);*/
    
    // colour = Albedo;

    ProcessOutputs(colour, alphaValue);
};

vec3 ProcessPointLight(PointLight light, vec3 WFP, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV){
    // constants
    vec3 lightDirection = normalize(light.position - WFP);
    vec3 H = normalize(VD + lightDirection);
    float dotNL = max(dot(N, lightDirection), 0.0);
    float dotNH = max(dot(N, H), 0.0);
    float dotVH = max(dot(VD, H), 0.0);

    // light radiance
    float dist = length(light.position - WFP);
    float attenuation = 1.0 / pow(dist, 2.0);
    vec3 radiance = light.colour * attenuation;

    // functions
    vec3 specularData[2];
    SpecularTerm(dotNH, A2, dotVH, RS, dotNV, 
                                    dotNL, R, specularData);

    vec3 specular = specularData[0];
    vec3 ks = specularData[1];
    vec3 kd = vec3(1.0) - ks;  
    kd *= 1.0 - M;

    return (kd * AD + specular) * radiance * dotNL * light.brightness;
}
vec3 PointLights(PointLight pointLights[maxPointLights], int numberOfPointLights, vec3 WFP, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV) {
    vec3 accumlativeLight = vec3(0);
    for(int i = 0; i < numberOfPointLights; i++){
        accumlativeLight += ProcessPointLight(pointLights[i], WFP, VD, N, RS, AD, A2, R, M, dotNV);
    }
    return accumlativeLight;
}

vec3 ProcessDirectionalLight(DirectionalLight light, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV){
    // constants
    vec3 lightDirection = abs(normalize(light.direction));
    vec3 H = normalize(VD + lightDirection);
    float dotNL = max(dot(N, lightDirection), 0.0);
    float dotNH = max(dot(N, H), 0.0);
    float dotVH = max(dot(VD, H), 0.0);

    // functions
    vec3 specularData[2];
    SpecularTerm(dotNH, A2, dotVH, RS, dotNV, dotNL, R, specularData);

    vec3 specular = specularData[0];
    vec3 ks = specularData[1];
    vec3 kd = vec3(1.0) - ks;  
    kd *= 1.0 - M;

    return (kd * AD + specular) * light.colour * dotNL * light.brightness;
}

vec3 DirectionalLights(DirectionalLight directionalLights[maxDirectionalLights], int numberOfDirectionalLights, vec3 VD, vec3 N, vec3 RS, vec3 AD, float A2, float R, float M, float dotNV){
    vec3 accumlativeLight = vec3(0);
    for(int i = 0; i < numberOfDirectionalLights; i++){
        accumlativeLight += ProcessDirectionalLight(directionalLights[i], VD, N, RS, AD, A2, R, M, dotNV);
    }
    return accumlativeLight;
}

float CalculateShadowValue(vec4 LSFragmentPosition, vec3 Normal) {
    vec3 projCoords = LSFragmentPosition.xyz / LSFragmentPosition.w;
    projCoords = 0.5 * projCoords + 0.5;

    float currentDepth = projCoords.z;

    if (currentDepth > 1.0)
        return 0.0;

    float bias = max(0.05 * (1.0 - dot(Normal, ShadowCasterPosition)), 0.005);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9;

    return shadow;
}

void ProcessOutputs(vec3 colour, float alphaValue){
    FragColour = vec4(colour, alphaValue);    

    float brightness = dot(FragColour.rgb, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > 1.0)
        BrightColor = vec4(FragColour.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

float Distribution(float dotNH, float a2) {
    // Micro Geometry Distribution Function
    // GGX/Trowbridge approximation
    float denominator = pow(dotNH, 2.0) * (a2 - 1.0) + 1.0;
    denominator = pow(denominator, 2.0);
    denominator *= PI;
    return a2 / max(denominator, 0.0000001);
};

vec3 Reflectance(float cosTheta, vec3 F0) {
    // Fresnel Reflectance Function
    // Schlick approximation
    return F0 + (1.0 - F0) * pow(max(1 - cosTheta, 0.0), 5.0);
};

float Geometry(float dotNV, float dotNL, float Roughness) {
    // Geometry Function
    // Schlick approximation
    float r = Roughness + 1.0;
    float k = (r * r) * 0.125; // div by 8
    float g1 = dotNV / (dotNV * (1.0 - k) + k);
    float g2 = dotNL / (dotNL * (1.0 - k) + k);
    return g1 * g2;
};

float CorrectionFactor(float dotNV, float dotNL) {
    return max(4.0 * dotNV * dotNL, 0.001);
};

void SpecularTerm(float dotNH, float a2, float cosTheta, vec3 F0, float dotNV, 
                float dotNL, float Roughness, inout vec3 info[2]) {
    float D = Distribution(dotNH, a2);
    float G = Geometry(dotNV, dotNL, Roughness);
    vec3 F = Reflectance(cosTheta, F0);
    vec3 numerator = D * G * F;
    info[0] = numerator / CorrectionFactor(dotNV, dotNL);
    info[1] = F;
};

vec3 getData(MatItem4 item) {
    return mix(item.raw, texture(item.id, TextureCoords), vec4(item.mixValue)).rgb;
}
vec3 getData(MatItem3 item) {
    return mix(item.raw, texture(item.id, TextureCoords).rgb, vec3(item.mixValue));
}
vec3 getNormal(MatItem3 item, vec3 normal) {
    return mix(normal, texture(item.id, TextureCoords).rgb, vec3(item.mixValue));
}
float getData(MatItem1 item) {
    return mix(item.raw, texture(item.id, TextureCoords).r, item.mixValue);
}
float getAlpha(MatItem4 item) {
    return mix(item.raw, texture(item.id, TextureCoords), vec4(item.mixValue)).a;
}
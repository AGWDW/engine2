#version 460 core
precision highp float;

out vec4 FragColor;

in VS_OUT {
    vec3 normals;
    vec2 texCoords;
    vec3 worldPos;
    vec3 camPos;
    vec4 ws;
    vec4 lsPos;
} fs_in;

struct Material {
     vec4 albedo_vec;
     sampler2D albedo_id;

     vec4 normal_vec;
     sampler2D normal_id;

     vec4 metalic_vec;
     sampler2D metalic_id;
     
     vec4 roughness_vec;
     sampler2D roughness_id;

     vec4 ao_vec;
     sampler2D ao_id;
};

uniform Material material;
uniform samplerCube hdrMap;
uniform samplerCube lbrMap;
uniform sampler2D brdfLUT;
uniform sampler2D shadowMap;

const float PI = 3.14159265359f;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.

vec3 getData(vec4 col, sampler2D id){
    if (col.a == 0){
        return texture(id, fs_in.texCoords).rgb;
    }
    return col.rgb;
}
float getAlpha(vec4 col, sampler2D id){
    if(col.a == 0){
        return texture(id, fs_in.texCoords).a;
    }
    return col.a;
}
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}
vec3 norm_;
vec3 getNormalFromMap()
{
    vec3 tangentNormal = norm_ * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.worldPos);
    vec3 Q2  = dFdy(fs_in.worldPos);
    vec2 st1 = dFdx(fs_in.texCoords);
    vec2 st2 = dFdy(fs_in.texCoords);

    vec3 N  = normalize(fs_in.normals);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a2 = roughness * roughness * roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom *= PI * denom;

    return a2 / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    float denom = NdotV * (1.0 - k) + k;

    return NdotV / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    //cosTheta = min(cosTheta, 0);
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  
// ----------------------------------------------------------------------------
void main()
{
    /*vec3 c = fs_in.ws.xyz;
    c = normalize(c);
    FragColor = vec4(c, 1);
    return;*/
    vec3 albedo = pow(getData(material.albedo_vec, material.albedo_id), vec3(2.2));
    
    float metallic = getData(material.metalic_vec, material.metalic_id).r;

    float roughness = getData(material.roughness_vec, material.roughness_id).r;

    float ao = getData(material.ao_vec, material.ao_id).r;

    vec3 N = normalize(Normal);
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);
	           
    // reflectance equation
    vec3 lightPositions[1] = {
        vec3(0, 1 ,0.5)
    };
    vec3 lightColors[1] = {
        vec3(1, 1, 1)
    };
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 1; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lightPositions[i] - WorldPos);
        vec3 H = normalize(V + L);
        float distance    = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColors[i] * attenuation;        
        
        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);        
        float G   = GeometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  
            
        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);                
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
    }   
  
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    FragColor = vec4(color, 1.0);
}
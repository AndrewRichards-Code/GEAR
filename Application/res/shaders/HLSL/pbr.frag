#version 450 core

//To Post-Processing
layout(location = 0) out vec4 colour;

//From Vertex Shader
layout(location = 0) in vec4 v_Position;
layout(location = 1) in vec2 v_TextCoord;
layout(location = 2) in float v_TextIds;
layout(location = 3) in vec4 v_Normal;
layout(location = 4) in vec4 v_WorldSpace;
layout(location = 5) in vec4 v_VertexToCamera;
layout(location = 6) in vec4 v_Colour;

//From Application
layout(binding = 0) uniform samplerCube u_Texture;
layout(binding = 1) uniform sampler2D u_Textures[32];

struct Light
{
	vec4  Colour;
	vec4  Position;
	vec4  Direction;
	float Type;
	float ShineFactor;
	float Reflectivity;
	float AmbientFactor;
	float AttenuationConstant;
	float AttenuationLinear;
	float AttenuationQuadratic;
	float CutOff; 
};
const int MAX_LIGHTS = 8;

layout(std140, binding = 2) uniform lightUBO
{
	Light[MAX_LIGHTS] u_Lights;
};

layout(std140, binding = 4) uniform PBRInfoUBO
{
	vec4 u_F0;
	vec4 u_Albedo;
	float u_Metallic;
	float u_Roughness;
	float u_AO;
	float _pad;
};
	
layout(binding = 5) uniform sampler2D u_TextureAlbedo;
layout(binding = 6) uniform sampler2D u_TextureMetallic;
layout(binding = 7) uniform sampler2D u_TextureRoughness;
layout(binding = 8) uniform sampler2D u_TextureAO;
layout(binding = 9) uniform sampler2D u_TextureNormal;
layout(binding = 10) uniform samplerCube u_TextureEnvironment;

//Functions
const float pi = 3.14151926535;
const float gamma = 2.2;

vec4 GetAlbedo()
{
	return pow(texture(u_TextureAlbedo, v_TextCoord), vec4(gamma));
}
float GetMetallic()
{
	return pow(texture(u_TextureMetallic, v_TextCoord).x, gamma);
}
float GetRoughness()
{
	return pow(texture(u_TextureRoughness, v_TextCoord).x, gamma);
}
float GetSmothness()
{	
	return 1.0 - GetRoughness();
}
float GetAO()
{
	return pow(texture(u_TextureAO, v_TextCoord).x, gamma);
}
vec4 GetNormals()
{
	vec4 normalMap = texture(u_TextureNormal, v_TextCoord) * 2.0 - 1.0;
	normalMap = v_Normal * normalMap;
	normalMap = normalize(normalMap);
	return normalMap;
}
vec3 FinalGamma(vec3 colour)
{
	return pow(colour, vec3(1.0 / gamma));
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = pi * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 CalcPBRLighting(int i)
{
	vec3 lightOut = {0, 0, 0};
	
	vec4 unitNormal = normalize(GetNormals());
	vec4 unitVertexToCamera = normalize(v_VertexToCamera);
	vec4 unitVertexToLight = normalize(u_Lights[i].Position - v_WorldSpace);
	vec4 unitHalfVector = normalize(unitNormal + unitVertexToCamera);

	float distanceVertToCam = length(unitVertexToCamera);
	float attenuation =  1.0 / (distanceVertToCam * distanceVertToCam);
	vec3 radiance = u_Lights[i].Colour.xyz * attenuation;

	vec3 F0 = mix(u_F0.xyz, GetAlbedo().xyz, GetMetallic());
	vec3 F = FresnelSchlickRoughness(max(dot(unitHalfVector, unitVertexToCamera), 0.0), F0, GetRoughness());

	float NDF = DistributionGGX(unitNormal.xyz, unitHalfVector.xyz, GetRoughness());
	float G = GeometrySmith(unitNormal.xyz, unitVertexToCamera.xyz, unitVertexToLight.xyz, GetRoughness());

	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * (max(dot(unitNormal, unitVertexToCamera), 0.0) * max(dot(unitNormal, unitVertexToLight), 0.0));
	vec3 specular = numerator / max(denominator, 0.001); 


	vec3 kSpecular = F;
	vec3 kDiffused = vec3(1.0) - kSpecular;
	kDiffused *= 1.0 - GetMetallic();
	vec3 irradiance = texture(u_TextureEnvironment, unitNormal.xyz).xyz;
	vec3 diffuse = irradiance * GetAlbedo().xyz;
	vec3 ambient = kDiffused * diffuse * GetAO(); 

	lightOut = ((kDiffused  * GetAlbedo().xyz / pi) + specular) * radiance * max(dot(unitNormal, unitVertexToLight), 0.0);

	vec3 colourOut  = ambient + lightOut; 
	colourOut = FinalGamma(colourOut);

	return colourOut;
}

void main()
{
	vec3 lighting;
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		lighting += CalcPBRLighting(i);
	}
	colour = vec4(lighting, 1.0f) *  GetAlbedo();
}
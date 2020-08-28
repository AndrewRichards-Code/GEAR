#include "msc_common.h"

//To Post-Processing
struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

//From Vertex Shader
struct PS_IN
{
	MIRU_LOCATION(0, float4, position, SV_POSITION);
	MIRU_LOCATION(1, float2, texCoord, TEXCOORD1);
	MIRU_LOCATION(2, float3x3, tbn, MATRIX2);
	MIRU_LOCATION(6, float4, worldSpace, POSITION6);
	MIRU_LOCATION(7, float4, vertexToCamera, POSITION7);
	MIRU_LOCATION(8, float4, colour, COLOR8);
};

//From Application

struct PBRConstants
{
	float4 albedo;
	float4 diffuse;
	float metallic;
	float roughness;
	float ambientOcclusion;
	float pad;
	float4 emissive;
};
MIRU_UNIFORM_BUFFER(1, 1, PBRConstants, pbrConsts);

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 2, float4, normal);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 3, float4, albedo);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 4, float4, metalness);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 5, float4, roughness);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 6, float4, ambientOcclusion);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 1, 7, float4, emissive);

struct Light
{
	float4 colour;
	float4 position;
	float4 direction;
};
static const uint MAX_LIGHTS = 8;

struct Lights
{
	Light lights[8];
}; 
MIRU_UNIFORM_BUFFER(2, 0, Lights, lights);

//Helper Functions
static const float PI = 3.1415926535897932384626433832795;

float3 GetNormal(PS_IN IN)
{
	float3 N = normal_image_cis.Sample(normal_sampler_cis, IN.texCoord).xyz;
	N = normalize(2.0 * N - 1.0);
	return normalize(mul(IN.tbn, N));
}
float3 GetAlbedo(PS_IN IN) 
{ 
	return pbrConsts.diffuse.rgb * albedo_image_cis.Sample(albedo_sampler_cis, IN.texCoord).rgb; 
}
float GetMetallic(PS_IN IN) 
{ 
	return pbrConsts.metallic * metalness_image_cis.Sample(metalness_sampler_cis, IN.texCoord).r; 
}
float GetRoughness(PS_IN IN) 
{
	return pbrConsts.roughness * roughness_image_cis.Sample(roughness_sampler_cis, IN.texCoord).r; 
}
float GetAmbientOcclusion(PS_IN IN) 
{
	return pbrConsts.ambientOcclusion * ambientOcclusion_image_cis.Sample(ambientOcclusion_sampler_cis, IN.texCoord).r; 
}
float3 GetEmissive(PS_IN IN) 
{
	return pbrConsts.emissive.rgb * emissive_image_cis.Sample(emissive_sampler_cis, IN.texCoord).rgb; 
}

float3 FresnelSchlick(float3 F0, float3 Wo, float3 N)
{
	return F0 + (1.0 - F0) * pow(1.0 - max(dot(Wo, N),0.0), 5);
}

//GGX/Towbridge-Reitz normal distribution function. Uses Disney's reparametrization of alpha = roughness^2.
float GGXTR(float cosWhN, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;
	float denom = (cosWhN * cosWhN) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX
float SchlickGGXSub(float cosWoN, float k)
{
	return cosWoN / ((cosWoN * (1 - k)) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float SchlickGGX(float cosWiN, float cosWoN, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return SchlickGGXSub(cosWiN, k) * SchlickGGXSub(cosWoN, k);
}

PS_OUT main(PS_IN IN)
{
	PS_OUT OUT;
	
	//Material Properties
	float3 N = GetNormal(IN);
	float3 albedo = GetAlbedo(IN);
	float metallic = GetMetallic(IN);
	float roughness = GetRoughness(IN);
	float ambientOcclusion = GetAmbientOcclusion(IN);
	float3 emissive = GetEmissive(IN);
	
	//Output light vector in the direction of the camera
	float3 Wo = IN.vertexToCamera.xyz;
	
	//Cosine of the angle between Wo and N
	float cosWo = max(dot(Wo, N), 0.0);
	
	//Fresnel reflectance
	const float3 Fdielectric = float3(0.04, 0.04, 0.04);
	float3 F0 = lerp(Fdielectric, albedo, metallic);
	
	//Direct Lights
	float3 Lo = 0.0;
	for(uint i = 0; i < MAX_LIGHTS; i++)
	{
		//Light Properties
		Light light = lights.lights[i];
		//Input light vector(retro)
		float3 Wi = /*-light.direction.xyz;*/light.position.xyz -IN.worldSpace.xyz;
		float WiDistance = length(light.position.xyz - IN.worldSpace.xyz);
		float attenuation = 1.0 / (WiDistance * WiDistance);
		float3 Li = light.colour.rgb * attenuation;
		
		//Half vector between input and output vector
		float3 Wh = normalize(Wo + Wi);
		
		//Cosine of the angle between Wi and N
		float cosWi = max(dot(Wi, N), 0.0);
		//Cosine of the angle between Wh and N
		float cosWh = max(dot(Wh, N), 0.0);
		
		//Calculate Fresnel
		float3 F = FresnelSchlick(F0, Wo, Wh);
		//Calculate Normal Distrubtion of Spectral BRDF
		float3 D = GGXTR(cosWh, roughness);
		//Calculate Geometry Factor of Spectral BRDF
		float3 G = SchlickGGX(cosWi, cosWo, roughness);

		//Calculat Diffuse component, kD + F0 = 1.0
		float3 kD = float3(1.0, 1.0, 1.0) - F0;
		//Metal have no diffuse, so adjust for metallic
		kD *= 1.0 - metallic;
		
		//Final Diffuse and Specular BRDF
		float3 diffuse = kD * albedo / PI;
		float3 specular = (F * D * G) /max((4.0 * cosWi * cosWo), 0.000001);
		float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ambientOcclusion;
		
		//Final light contribution
		Lo += emissive + ambient + ((diffuse + specular) * Li * cosWi);
	}
	
	// Ambient lighting (IBL).
	{
	}
	
	OUT.colour = float4(Lo, 1.0);
	return OUT;
}
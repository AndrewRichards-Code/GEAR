#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "PBRFunctions.h"
#include "../Depth.h"
#include "../CubeFunctions.h"

struct VS_IN
{
	MIRU_LOCATION(0, float4, position, POSITION0);
	MIRU_LOCATION(1, float2, texCoord, TEXCOORD1);
	MIRU_LOCATION(2, float4, normal, NORMAL2);
	MIRU_LOCATION(3, float4, tangent, TANGENT3);
	MIRU_LOCATION(4, float4, binormal, BINORMAL4);
	MIRU_LOCATION(5, float4, colour, COLOR5);
};

struct VS_OUT
{
	MIRU_LOCATION(0, float4, position, SV_POSITION);
	MIRU_LOCATION(1, float2, texCoord, TEXCOORD1);
	MIRU_LOCATION(2, float3x3, tbn, MATRIX2);
	MIRU_LOCATION(6, float4, worldPosition, POSITION6);
	MIRU_LOCATION(7, float4, viewPosition, POSITION7);
	MIRU_LOCATION(8, float4, vertexToCamera, POSITION8);
	MIRU_LOCATION(9, float4, colour, COLOR9);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);
MIRU_UNIFORM_BUFFER(0, 1, Lights, lights);
MIRU_UNIFORM_BUFFER(0, 2, ProbeInfo, probeInfo);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 3, float4, diffuseIrradiance);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 4, float4, specularIrradiance);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 0, 5, float4, specularBRDF_LUT);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D_ARRAY, 0, 6, float4, shadowMap2DArray);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 7, float4, shadowMapCube);

MIRU_UNIFORM_BUFFER(1, 0, Model, model);

MIRU_UNIFORM_BUFFER(2, 0, PBRConstants, pbrConstants);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 1, float4, normal);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 2, float4, albedo);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 3, float4, metallic);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 4, float4, roughness);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 5, float4, ambientOcclusion);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 6, float4, emissive);

static const uint MAX_LIGHTS = 8;

VS_OUT vs_main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.position = mul(IN.position, mul(model.modl, mul(camera.view, camera.proj)));
	OUT.texCoord = float2(model.texCoordScale0.x * IN.texCoord.x, model.texCoordScale0.y * IN.texCoord.y);
	OUT.tbn = float3x3(mul(IN.tangent, model.modl).xyz, mul(IN.binormal, model.modl).xyz, mul(IN.normal, model.modl).xyz);
	OUT.worldPosition = mul(IN.position, model.modl);
	OUT.viewPosition = mul(OUT.worldPosition, camera.view);
	OUT.vertexToCamera = normalize(camera.position - OUT.worldPosition);
	OUT.colour = IN.colour;
	
	return OUT;
}

//Helper functions:
float3 GetNormal(PS_IN IN)
{
	float3 N = normal_ImageCIS.Sample(normal_SamplerCIS, IN.texCoord).xyz;
	N = normalize(2.0 * N - 1.0);
	return normalize(mul(N, IN.tbn));
}
float3 GetAlbedo(PS_IN IN) 
{ 
	return pbrConstants.albedo.rgb * albedo_ImageCIS.Sample(albedo_SamplerCIS, IN.texCoord).rgb; 
}
float GetMetallic(PS_IN IN) 
{ 
	return pbrConstants.metallic * metallic_ImageCIS.Sample(metallic_SamplerCIS, IN.texCoord).r; 
}
float GetRoughness(PS_IN IN) 
{
	return pbrConstants.roughness * roughness_ImageCIS.Sample(roughness_SamplerCIS, IN.texCoord).r; 
}
float GetAmbientOcclusion(PS_IN IN) 
{
	return pbrConstants.ambientOcclusion * ambientOcclusion_ImageCIS.Sample(ambientOcclusion_SamplerCIS, IN.texCoord).r; 
}
float3 GetEmissive(PS_IN IN) 
{
	return pbrConstants.emissive.rgb * emissive_ImageCIS.Sample(emissive_SamplerCIS, IN.texCoord).rgb; 
}

float ShadowPCF(float2 shadowTextureCoords, uint shadowCascadeIndex, float lightSpaceZ)
{
	float shadowStrength = 0.0;
	float bias = 0.0001;
	int range = 2;
	int size = 2 * range + 1;
	int count = size * size;
	
	uint3 shadowMap2DArray_ImageCIS_Dim;
	shadowMap2DArray_ImageCIS.GetDimensions(shadowMap2DArray_ImageCIS_Dim.x, shadowMap2DArray_ImageCIS_Dim.y, shadowMap2DArray_ImageCIS_Dim.z);
	float scale = 0.75;
	float dx = scale * (1.0 / float(shadowMap2DArray_ImageCIS_Dim.x));
	float dy = scale * (1.0 / float(shadowMap2DArray_ImageCIS_Dim.y));
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			float2 offset = float2(dx * x, dy * y);
			float shadowDepth = shadowMap2DArray_ImageCIS.SampleLevel(shadowMap2DArray_SamplerCIS, float3(shadowTextureCoords + offset, shadowCascadeIndex), 0.0).x;
			
			if (shadowDepth - bias > lightSpaceZ) //Reverse Z
			{
				shadowStrength += 0.0;
			}
			else
			{
				shadowStrength += 1.0;
			} 
		}
	}
	shadowStrength /= count;
	
	return shadowStrength;
}

float GetShadowStrength(float4 worldPosition, float viewPositionZ, Light light)
{
	float shadowStrength = 1.0;
	float bias = 0.00001;
	float lightSpaceZ = 1.0;
	float shadowDepth = 0.0;
	
	float type = light.type_valid_spotInner_spotOuter.x;
	
	uint shadowCascadeIndex = 0;
	for (uint i = 0; i < probeInfo.shadowCascades; i++)
	{
		if (viewPositionZ < probeInfo.farPlanes[i])
		{
			shadowCascadeIndex = i;
			break;
		}
	}
	
	if (type == 0.0) //POINT
	{
		float3 lightDirection = worldPosition.xyz - light.position.xyz;
		uint faceID = UVWToFaceIndex(lightDirection);
		float4 lightSpaceProjectedPosition = PerspectiveDivide(worldPosition, probeInfo.view[faceID], probeInfo.proj[0]);
		lightSpaceZ = lightSpaceProjectedPosition.z;
		shadowDepth = shadowMapCube_ImageCIS.SampleLevel(shadowMapCube_SamplerCIS, lightDirection, 0.0).x;
	}
	else if (type == 1.0) //DIRECTIONAL
	{
		float4 lightSpaceProjectedPosition = PerspectiveDivide(worldPosition, probeInfo.view[shadowCascadeIndex], probeInfo.proj[shadowCascadeIndex]);
		float2 shadowTextureCoords = (lightSpaceProjectedPosition.xy / 2.0) + float2(0.5, 0.5);
		lightSpaceZ = lightSpaceProjectedPosition.z;
		
		if (lightSpaceProjectedPosition.w > 0.0 && lightSpaceProjectedPosition.z > -1.0 && lightSpaceProjectedPosition.z < 1.0)
			return ShadowPCF(shadowTextureCoords, shadowCascadeIndex, lightSpaceZ);
		else
			return shadowStrength;
		
		if (viewPositionZ > probeInfo.farPlanes[probeInfo.shadowCascades - 1]) //Beyond furthest cascade
			return shadowStrength;
	}	
	else if (type == 2.0) //SPOT
	{
		float4 lightSpaceProjectedPosition = PerspectiveDivide(worldPosition, probeInfo.view[0], probeInfo.proj[0]);
		float2 shadowTextureCoords = (lightSpaceProjectedPosition.xy / 2.0) + float2(0.5, 0.5);
		lightSpaceZ = lightSpaceProjectedPosition.z;
		shadowDepth = shadowMap2DArray_ImageCIS.SampleLevel(shadowMap2DArray_SamplerCIS, float3(shadowTextureCoords, 0), 0.0).x;
	}
	
	if (shadowDepth - bias > lightSpaceZ) //Reverse Z
	{
		shadowStrength = 0.0;
	}
	return shadowStrength;
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	
	//Material Properties
	float3 N = GetNormal(IN);
	float3 albedo = GetAlbedo(IN);
	float metallic = GetMetallic(IN);
	float roughness = GetRoughness(IN);
	float ambientOcclusion = GetAmbientOcclusion(IN);
	float3 emissive = GetEmissive(IN);
	
	//Output light vector in the direction of the camera.
	float3 Wo = IN.vertexToCamera.xyz;
	
	//Cosine of the angle between Wo and N.
	float cosWo = max(dot(Wo, N), 0.0);
	
	//Fresnel reflectance.
	//const float3 Fdielectric = float3(0.04, 0.04, 0.04);
	float3 Fdielectric = pbrConstants.fresnel.rgb;
	float3 F0 = lerp(Fdielectric, albedo, metallic);
	
	//Direct Lights
	float3 Lo = 0.0;
	for(uint i = 0; i < MAX_LIGHTS; i++)
	{
		//Light Properties.
		Light light = lights.lights[i];
		
		if (light.type_valid_spotInner_spotOuter.y == 0.0)
			continue;
		
		//Input light vector(retro).
		float3 Wi;
		if (light.type_valid_spotInner_spotOuter.x == 0.0
			|| light.type_valid_spotInner_spotOuter.x == 2.0) //POINT or SPOT
		{
			Wi = light.position.xyz - IN.worldPosition.xyz;
		}
		else //DIRECTIONAL
		{
			Wi = normalize(-light.direction.xyz);
		}
		
		//Attenuation
		float WiDistance = length(Wi);
		float attenuation = 1.0 / (WiDistance * WiDistance);
		
		//Input Radiance
		float3 Li = normalize(light.colour.rgb) * light.colour.a * attenuation;
		
		//Calculate fade from inner to outer cone for spot.
		if (light.type_valid_spotInner_spotOuter.x == 2.0) //SPOT
		{
			float3 spotWi = -light.direction.xyz;
			float theta = dot(normalize(spotWi), normalize(Wi));
			float cosInner = cos(light.type_valid_spotInner_spotOuter.z);
			float cosOuter = cos(light.type_valid_spotInner_spotOuter.w);
			float epsilon = cosInner - cosOuter;
			float intensity = saturate(lerp(0.0, 1.0, ((theta - cosOuter) / max(epsilon, 0.000001))));
			Li *= intensity;
		}
		
		//Shadows
		float shadowStrength = GetShadowStrength(IN.worldPosition, IN.viewPosition.z, light);
		Li *= shadowStrength;
		
		//Half vector between input and output vector.
		float3 Wh = normalize(Wo + Wi);
		
		//Cosine of the angle between Wi and N.
		float cosWi = max(dot(Wi, N), 0.0);
		//Cosine of the angle between Wh and N.
		float cosWh = max(dot(Wh, N), 0.0);
		
		//Calculate Fresnel
		float3 F = FresnelSchlick(F0, Wo, Wh);
		//Calculate Normal Distrubtion of Spectral BRDF.
		float D = GGXTR(cosWh, roughness);
		//Calculate Geometry Factor of Spectral BRDF
		float G = SchlickGGX(cosWi, cosWo, roughness);

		//Calculate Diffuse component, kD + F0 = 1.0.
		float3 kD = float3(1.0, 1.0, 1.0) - F0;
		//Metal have no diffuse, so adjust for metallic.
		kD *= 1.0 - metallic;
		
		//Final Diffuse and Specular BRDF.
		float3 diffuse = kD * albedo / PI;
		float3 specular = (F * D * G) / max((4.0 * cosWi * cosWo), 0.000001);
		float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ambientOcclusion;
		
		//Final light contribution.
		Lo += emissive + ambient + ((diffuse + specular) * Li * cosWi);
	}
	
	// Ambient lighting (IBL).
	{
		//Diffuse:
		//Get difuse irradiance.
		float3 diffuseIrradiance = diffuseIrradiance_ImageCIS.Sample(diffuseIrradiance_SamplerCIS, N).rgb;
		
		//Get Fresnel term at the normal for diffuse contributions from ambient lighting.
		float3 F = FresnelSchlick(F0, Wo, N);
		
		//Get diffuse co-efficient
		float3 kD = lerp(1.0 - F, 0.0, metallic);
		
		//Calculate diffuse contribution using Lamberitian BRDF.
		float3 diffuseIBL = kD * albedo * diffuseIrradiance;
		
		//Specular:
		// Specular reflection vector.
		float3 Wr = reflect(-Wo, N);//2.0 * max(dot(N, Wo), 0.0) * N - Wo;
		
		//Get specular irradiance at correction mipmap level.
		uint width, height, levels;
		specularIrradiance_ImageCIS.GetDimensions(0, width, height, levels);
		float3 specularIrradiance = specularIrradiance_ImageCIS.SampleLevel(specularIrradiance_SamplerCIS, Wr, roughness * levels).rgb;
		
		//Get specular BDRF.
		float2 specularBRDF	= specularBRDF_LUT_ImageCIS.Sample(specularBRDF_LUT_SamplerCIS, float2(max(dot(N, Wo), 0.0), roughness)).rg;
	
		//Calculate specular contribution using Split-sum approximation for Cook-Torrance.
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
		
		//Final Light contribution.
		Lo += diffuseIBL + specularIBL;
	}
	
	OUT.colour = float4(Lo, 1.0);
	
	return OUT;
}
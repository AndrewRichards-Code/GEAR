#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "PBRFunctions.h"
#include "../CubeFunctions.h"

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 0, float4, environment);
MIRU_RW_IMAGE_2D_ARRAY(0, 1, float4, specularIrradiance);
MIRU_UNIFORM_BUFFER(0, 2, SpecularIrradianceInfo, specularIrradianceInfo);

static const uint NumSamples = 1024;

MIRU_COMPUTE_LAYOUT(32, 32, 1)
void main(uint3 threadID : MIRU_DISPATCH_THREAD_ID)
{
	//Ensure output is bounded when computing higher mipmap levels.
	uint3 specularIrradianceDim;
	specularIrradiance.GetDimensions(specularIrradianceDim.x, specularIrradianceDim.y, specularIrradianceDim.z);
	if (threadID.x >= specularIrradianceDim.x || threadID.y >= specularIrradianceDim.y)
		return;
	
	if (specularIrradianceInfo.roughness == 0.0) // Copy the top mip
	{
		specularIrradiance[threadID] = environment_ImageCIS.SampleLevel(environment_SamplerCIS, GetLookupUVW(threadID, float2(specularIrradianceDim.xy)), 0.0);
		return;
	}
	
	//Get environment dimensions.
	float3 environmentDim;
	environment_ImageCIS.GetDimensions(0, environmentDim.x, environmentDim.y, environmentDim.z);

	//Solid angle associated with a single cubemap texel.
	float texelSolidAngle = 4.0 * PI / (6 * environmentDim.x * environmentDim.y);
	
	//Get Normal vector.
	float3 N = GetLookupUVW(threadID, specularIrradianceDim.xy);
	float3 Wo = N;
	
	//Calculate TBN vectors.
	float3 B, T;
	ComputeBasisVectors(N, B, T);
	
	float3 colour = 0;
	float weight = 0;
	
	//Convolve environment map using GGX NDF importance sampling.
	//Weight by cosine term since Epic claims it generally improves quality.
	for (uint i = 0; i < NumSamples; i++)
	{
		//Get Hammersley sample and sample GGX in tangent-space t get 'random' halfway vector.
		float3 Wh = TangentToWorld(SampleGGX(SampleHammersley(i, NumSamples), specularIrradianceInfo.roughness), N, B, T);

		//Calculate incident direction (Wi) by reflecting viewing direction (Wo) around half-vector (Wh).
		float3 Wi = 2.0 * dot(Wo, Wh) * Wh - Wo;
		
		float cosWiN = dot(Wi, N);
		if (cosWiN > 0.0) 
		{
			//Use Mipmap Filtered Importance Sampling to improve convergence.
			//See: https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html, section 20.4

			float cosWhN = max(dot(Wh, N), 0.0);

			//GGX normal distribution function (D term) probability density function.
			//Scaling by 1/4 is due to change of density in terms of Wh to Wi (and since N = V, rest of the scaling factor cancels out).
			float D = GGXTR(cosWhN, specularIrradianceInfo.roughness) * 0.25;

			//Solid angle associated with this sample.
			float sampleSolidAngle = 1.0 / (NumSamples * D);

			//Mipmap level to sample from.
			float mipLevel = max(0.5 * log2(sampleSolidAngle / texelSolidAngle) + 1.0, 0.0);

			colour += environment_ImageCIS.SampleLevel(environment_SamplerCIS, Wi, mipLevel).rgb * cosWiN;
			weight += cosWiN;
		}
	}
	colour /= weight;

	specularIrradiance[threadID] = float4(colour, 1.0);
}
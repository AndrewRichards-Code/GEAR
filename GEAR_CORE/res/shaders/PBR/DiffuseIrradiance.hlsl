#include "msc_common.h"
#include "PBRFunctions.h"
#include "../CubeFunctions.h"

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 0, float4, environment);
MIRU_RW_IMAGE_2D_ARRAY(0, 1, float4, diffuseIrradiance);

static const uint NumSamples = 64 * 1024;

MIRU_COMPUTE_LAYOUT(32, 32, 1)
void main(uint3 threadID : MIRU_DISPATCH_THREAD_ID)
{
	//Get Normal vector.
	float3 diffuseIrradianceDim;
	diffuseIrradiance.GetDimensions(diffuseIrradianceDim.x, diffuseIrradianceDim.y, diffuseIrradianceDim.z);
	
	//Calculate TBN vectors.
	float3 N = GetLookupUVW(threadID, diffuseIrradianceDim.xy);
	float3 B, T;
	ComputeBasisVectors(N, B, T);
	
	float3 irradiance = 0.0;
	
	//Sample diffuse irradiance over the hemisphere.
	for (uint i = 0; i < NumSamples; i++) 
	{
		//Get Input direction in world space from hemisphere using hammersley. 
		float3 Wi = TangentToWorld(SampleHemisphere(SampleHammersley(i, NumSamples)), N, B, T);
		float cosTheta = max(dot(Wi, N), 0.0);

		//PIs here cancel out because of division by pdf.
		irradiance += 2.0 * environment_ImageCIS.SampleLevel(environment_SamplerCIS, Wi, 0).rgb * cosTheta;
	}
	irradiance /= float(NumSamples);
	
	diffuseIrradiance[threadID] = float4(irradiance, 1.0);
}
#include "msc_common.h"
#include "PBRFunctions.h"

MIRU_RW_IMAGE_2D(0, 0, float4, brdf_lut);

static const uint NumSamples = 1024;

MIRU_COMPUTE_LAYOUT(32, 32, 1)
void main(uint3 threadID : MIRU_DISPATCH_THREAD_ID)
{
	//Get output BRDF_LUT dimensions.
	float width, height;
	brdf_lut.GetDimensions(width, height);
	
	//Get integration parameters.
	float cosWoN = max(threadID.x / width, 0.00001); //cosWoN must > 0.0.
	float roughness = threadID.y / height;
	
	//Derive tangent-space viewing vector from angle to normal (pointing towards +Z in this reference frame).
	float3 Wo = float3(sqrt(1.0 - cosWoN * cosWoN), 0.0, cosWoN);
	
	//DFG1 & DFG2 are terms of split-sum approximation of the reflectance integral.
	float DFG1 = 0;
	float DFG2 = 0;
	
	for (uint i = 0; i < NumSamples; i++)
	{
		//Get Hammersley sample and sample GGX in tangent-space t get 'random' halfway vector.
		float3 Wh = SampleGGX(SampleHammersley(i, NumSamples), roughness);
		
		//Calculate incident direction (Wi) by reflecting viewing direction (Wo) around half-vector (Wh).
		float3 Wi = 2.0 * dot(Wo, Wh) * Wh - Wo;
		
		float cosWi   = Wi.z;
		float cosWh   = Wh.z;
		float cosWoWh = max(dot(Wo, Wh), 0.0);

		if (cosWi > 0.0) 
		{
			float G  = SchlickGGX_IBL(cosWi, cosWoN, roughness);
			float Gv = G * cosWoWh / (cosWoN * cosWoN);
			float Fc = pow(1.0 - cosWoWh, 5);

			DFG1 += (1 - Fc) * Gv;
			DFG2 += Fc * Gv;
		}
	}
	
	brdf_lut[threadID.xy] = float2(DFG1, DFG2) / NumSamples;
}
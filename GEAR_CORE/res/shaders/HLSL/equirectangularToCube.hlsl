#include "msc_common.h"

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 0, 0, float4, inputImage);
MIRU_RW_IMAGE_2D_ARRAY(0, 1, float4, outputImage);

static const float PI = 3.1415926535897932384626433832795;

float3 GetLookupUVW(uint3 threadID)
{
	float3 outputImageDim;
	outputImage.GetDimensions(outputImageDim.x, outputImageDim.y, outputImageDim.z);
	
	float2 faceTC = threadID.xy / outputImageDim.xy;
	float2 uv = 2.0 * faceTC - float2(1.0, 1.0);
	
	// Select vector based on cubemap face index.
	float3 ret;
	
	switch(threadID.z)
	{
	case 0: ret = float3(1.0,  uv.y, -uv.x); break;
	case 1: ret = float3(-1.0, uv.y,  uv.x); break;
	case 2: ret = float3(uv.x, 1.0, -uv.y); break;
	case 3: ret = float3(uv.x, -1.0, uv.y); break;
	case 4: ret = float3(uv.x, uv.y, 1.0); break;
	case 5: ret = float3(-uv.x, uv.y, -1.0); break;
	}
	
	return normalize(ret);
}

float2 CubemapToEquirectangularTextCoords(float3 v)
{
	float3 norm_v = normalize(v);
	float theta = asin(norm_v.y);
	float phi = atan2(norm_v.z, norm_v.x);
	float2 uv = float2(phi/(2 * PI), -((theta/PI) + 0.5));
	return uv;
}

MIRU_COMPUTE_LAYOUT(32, 32, 1)
void main(uint3 threadID : MIRU_DISPATCH_THREAD_ID)
{
	float3 v = GetLookupUVW(threadID);
	float2 texCoords = CubemapToEquirectangularTextCoords(v);
	outputImage[threadID] = inputImage_ImageCIS.SampleLevel(inputImage_SamplerCIS, texCoords, 0);
}
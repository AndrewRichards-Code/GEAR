#include "msc_common.h"
#include "CubeFunctions.h"

MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 0, 0, float4, equirectangularImage);
MIRU_RW_IMAGE_2D_ARRAY(0, 1, float4, cubeImage);

MIRU_COMPUTE_LAYOUT(32, 32, 1)
void main(uint3 threadID : MIRU_DISPATCH_THREAD_ID)
{
	float3 cubeImageDim;
	cubeImage.GetDimensions(cubeImageDim.x, cubeImageDim.y, cubeImageDim.z);
	
	float3 v = GetLookupUVW(threadID, cubeImageDim.xy);
	float2 texCoords = CubemapToEquirectangularTextCoords(v);
	cubeImage[threadID] = equirectangularImage_ImageCIS.SampleLevel(equirectangularImage_SamplerCIS, texCoords, 0);
}
#include "msc_common.h"

MIRU_IMAGE_2D(0, 0, float4, inputImage);
MIRU_RW_IMAGE_2D(0, 1, float4, outputImage);

MIRU_IMAGE_2D_ARRAY(0, 0, float4, inputImageArray);
MIRU_RW_IMAGE_2D_ARRAY(0, 1, float4, outputImageArray);

float4 ColateColour(uint2 threadID)
{
	uint3 pixel = uint3(2 * threadID.x, 2* threadID.y, 0);
	
	float4 colour00 = inputImage.Load(pixel, int2(0, 0));
	float4 colour01 = inputImage.Load(pixel, int2(0, 1));
	float4 colour10 = inputImage.Load(pixel, int2(1, 0));
	float4 colour11 = inputImage.Load(pixel, int2(1, 1));
	
	return (colour00 + colour01 + colour10 + colour11) / 4.0;
}

float4 ColateColourArray(uint3 threadID)
{
	int4 pixel = int4(2 * threadID.x, 2* threadID.y, threadID.z, 0);
	
	float4 colour00 = inputImageArray.Load(pixel, int2(0, 0));
	float4 colour01 = inputImageArray.Load(pixel, int2(0, 1));
	float4 colour10 = inputImageArray.Load(pixel, int2(1, 0));
	float4 colour11 = inputImageArray.Load(pixel, int2(1, 1));
	
	return (colour00 + colour01 + colour10 + colour11) / 4.0;
}

MIRU_COMPUTE_LAYOUT(8, 8, 1)
void mipmap(uint3 threadID : MIRU_DISPATCH_THREAD_ID)
{
	outputImage[threadID.xy] = ColateColour(threadID.xy);
}

MIRU_COMPUTE_LAYOUT(8, 8, 1)
void mipmap_array(uint3 threadID : MIRU_DISPATCH_THREAD_ID)
{
	outputImageArray[threadID] = ColateColourArray(threadID);
}

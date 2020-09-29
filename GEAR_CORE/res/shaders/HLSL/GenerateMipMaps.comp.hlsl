#include "msc_common.h"

/*struct MipMap
{
	uint SrcMipLevel;
	uint NumMipLevels;
	uint ImageDimension;
};
MIRU_UNIFORM_BUFFER(0, 0, MipMap, mipmap);*/

MIRU_SAMPLER(0, 0, samplerMip0);
MIRU_IMAGE_2D(0, 1, float4, imageMip0);
MIRU_RW_IMAGE_2D(0, 2, float4, imageMip1);

groupshared float4 gs_Colour[8][8];

float4 ColateColour(uint2 t)
{
	float4 colour00 = gs_Colour[t.x + 0][t.y + 0];
	float4 colour01 = gs_Colour[t.x + 0][t.y + 1];
	float4 colour10 = gs_Colour[t.x + 1][t.y + 0];
	float4 colour11 = gs_Colour[t.x + 1][t.y + 1];
	
	return (colour00 + colour01 + colour10 + colour11) / 4.0;
}

MIRU_COMPUTE_LAYOUT(8, 8, 1)
void main(uint3 t : MIRU_DISPATCH_THREAD_ID)
{
	gs_Colour[t.x][t.y] = imageMip0.SampleLevel(samplerMip0, float2(t.xy)/float2(2048.0, 2048.0), 0);
	
	GroupMemoryBarrierWithGroupSync();
	if(t.x % 2 || t.y % 2)
		return;
	
	imageMip1[t.xy/2] = float4(1, 1, 1, 1) + ColateColour(t.xy);
}

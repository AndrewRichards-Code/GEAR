#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "ColourFunctions.h"

MIRU_UNIFORM_BUFFER(0, 0, HDRInfo, hdrInfo);
MIRU_IMAGE_2D(0, 1, float4, hdrInput);

struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

VS_OUT vs_main(uint VertexIndex : SV_VertexID)
{
	VS_OUT OUT;
	OUT.v_Position = float4(float2((VertexIndex << 1) & 2, VertexIndex & 2) * 2.0f - 1.0f, 0.0f, 1.0f);
	return OUT;
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	
	//Calculate irradiance
	float4 irradiance = hdrInput.Load(int3(IN.v_Position.xy, 0));
	
	//Exposure tone mapping
	float3 mapped = float3(1.0, 1.0, 1.0) - exp(-irradiance.rgb * hdrInfo.exposure);
	
	//Convert Linear to sRGB colour space.
	OUT.colour.rgb = GammaCorrection(hdrInfo.gammaSpace, mapped);
	OUT.colour.a = 1.0;
	
	return OUT;
}

MIRU_RW_IMAGE_2D(0, 0, float4, emissiveIn);
MIRU_RW_IMAGE_2D(0, 0, float4, emissiveOut);

float GaussianDistrubtion2D(int2 pos, float sigma)
{
	const float PI = 3.1415926535897932384626433832795;
	float _2sigma2 = 2.0 * sigma * sigma;
	float coeffiecent = 1.0 / (_2sigma2 * PI);
	float power = -float((pos.x * pos.x) + (pos.y * pos.y)) / _2sigma2;
	return coeffiecent * exp(power);
}

MIRU_COMPUTE_LAYOUT(8, 8, 1)

void cs_GaussianBlur(uint3 pos : MIRU_DISPATCH_THREAD_ID)
{
	uint2 emissiveDim;
	emissiveIn.GetDimensions(emissiveDim.x, emissiveDim.y);

	int kernelSize = 5;
	float sigma = 5.5;
	int2 offset = int2(0, 0);
	int2 pos_xy = pos.xy + offset;
	float colour = emissiveIn.Load(int3(pos.xy, 0)) * GaussianDistrubtion2D(offset, sigma);
	
	for (int i = 1; i < kernelSize; i++)
	{
		offset = int2(+i, 0);
		pos_xy = pos.xy + offset;
		if (pos_xy.x >= 0 && pos_xy.x < emissiveDim.x)
			colour += emissiveIn.Load(int3(pos_xy, 0)) * GaussianDistrubtion2D(offset, sigma);
		
		offset = int2(-i, 0);
		pos_xy = pos.xy + offset;
		if (pos_xy.x >= 0 && pos_xy.x < emissiveDim.x)
			colour += emissiveIn.Load(int3(pos_xy, 0)) * GaussianDistrubtion2D(offset, sigma);
		
		offset = int2(0, +i);
		pos_xy = pos.xy + offset;
		if (pos_xy.y >= 0 && pos_xy.y < emissiveDim.y)
			colour += emissiveIn.Load(int3(pos_xy, 0)) * GaussianDistrubtion2D(offset, sigma);
		
		offset = int2(0, -i);
		pos_xy = pos.xy + offset;
		if (pos_xy.y >= 0 && pos_xy.y < emissiveDim.y)
			colour += emissiveIn.Load(int3(pos_xy, 0)) * GaussianDistrubtion2D(offset, sigma);
	}
	emissiveOut[pos.xy] = colour;
}
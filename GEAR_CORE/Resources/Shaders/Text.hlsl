#include "msc_common.h"
#include "UniformBufferStructures.h"

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
	MIRU_LOCATION(2, float4, normal, NORMAL2);
	MIRU_LOCATION(3, float4, tangent, TANGENT3);
	MIRU_LOCATION(4, float4, binormal, BINORMAL4);
	MIRU_LOCATION(5, float4, colour, COLOR5);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

MIRU_UNIFORM_BUFFER(0, 0, Camera, textCamera);
MIRU_UNIFORM_BUFFER(1, 0, Model, model);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 2, 0, float4, fontAtlas);

VS_OUT vs_main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.position = mul(IN.position, mul(model.modl, mul(textCamera.view, textCamera.proj)));
	OUT.texCoord = IN.texCoord;
	OUT.colour = IN.colour;
	
	return OUT;
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	
	if (IN.texCoord.x == -1.0 && IN.texCoord.y == -1.0)
	{
		OUT.colour = IN.colour;
	}
	else
	{
		float alpha = fontAtlas_ImageCIS.Sample(fontAtlas_SamplerCIS, IN.texCoord).r;
		float4 sampled = float4(1.0, 1.0, 1.0, alpha);
		OUT.colour = IN.colour * sampled;
	}
	
	return OUT;
}
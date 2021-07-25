#include "msc_common.h"
#include "UniformBufferStructures.h"

struct VS_IN
{
	MIRU_LOCATION(0, float4, positions, POSITION0);
	MIRU_LOCATION(1, float2, texCoords, TEXCOORD1);
	MIRU_LOCATION(2, float4, normals, NORMAL2);
	MIRU_LOCATION(3, float4, tangents, TANGENT3);
	MIRU_LOCATION(4, float4, binormals, BINORMAL4);
	MIRU_LOCATION(5, float4, colours, COLOR5);
};

struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float2, v_TexCoord, TEXCOORD1);
	MIRU_LOCATION(2, float4, v_Normal, NORMAL2);
	MIRU_LOCATION(3, float4, v_Tangent, TANGENT3);
	MIRU_LOCATION(4, float4, v_Binormal, BINORMAL4);
	MIRU_LOCATION(5, float4, v_Colour, COLOR5);
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
	
	OUT.v_Position = mul(mul(mul(transpose(textCamera.proj), transpose(textCamera.view)), transpose(model.modl)), IN.positions);
	OUT.v_TexCoord = IN.texCoords;
	OUT.v_Colour = IN.colours;
	
	return OUT;
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	
	if (IN.v_TexCoord.x == -1.0 && IN.v_TexCoord.y == -1.0)
	{
		OUT.colour = IN.v_Colour;
	}
	else
	{
		float alpha = fontAtlas_ImageCIS.Sample(fontAtlas_SamplerCIS, IN.v_TexCoord).r;
		float4 sampled = float4(1.0, 1.0, 1.0, alpha);
		OUT.colour = IN.v_Colour * sampled;
	}
	
	return OUT;
}
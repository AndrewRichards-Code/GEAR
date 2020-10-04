#include "msc_common.h"
#include "UniformBufferStructures.h"

struct VS_IN
{
	MIRU_LOCATION(0, float4, positions, POSITION0);
	MIRU_LOCATION(1, float2, textCoords, TEXCOORD1);
	MIRU_LOCATION(2, float, textIds, PSIZE);
	MIRU_LOCATION(4, float4, colours, COLOR4);
};

struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float2, v_TextCoord, TEXCOORD1);
	MIRU_LOCATION(2, float, v_TextIds, PSIZE2);
	MIRU_LOCATION(3, float4, v_Colour, COLOR3);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);
MIRU_COMBINED_IMAGE_SAMPLER_ARRAY(MIRU_IMAGE_2D, 1, 0, float4, u_Textures, 32);

VS_OUT vs_main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.v_Position = mul(IN.positions, camera.proj);
	OUT.v_TextCoord.x = IN.textCoords.x;
	OUT.v_TextCoord.y = 1.0 - IN.textCoords.y;
	OUT.v_TextIds = IN.textIds;
	OUT.v_Colour = IN.colours;
	
	return OUT;
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	
	float4 sampled = {0, 0, 0, 0};
	if( IN.v_TextIds > 0)
	{
		for (int i = 0; i < 32; i++)
		{
			int tid = int(IN.v_TextIds - 0.5);
			if(tid == i)
			{
				float alpha = u_Textures_ImageCIS[tid].Sample(u_Textures_SamplerCIS[tid], IN.v_TextCoord).r;
				sampled = float4(1.0, 1.0, 1.0, alpha);
			}
		}
	}
	OUT.colour = IN.v_Colour * sampled;
	
	return OUT;
}
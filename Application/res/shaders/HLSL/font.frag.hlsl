#include "msc_common.h"

//To Post-Processing
struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

//From Vertex Shader
struct PS_IN
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float2, v_TextCoord, TEXCOORD1);
	MIRU_LOCATION(2, float, v_TextIds, PSIZE2);
	MIRU_LOCATION(3, float4, v_Colour, COLOR3);
};

//From Application
MIRU_COMBINED_IMAGE_SAMPLER_ARRAY(MIRU_IMAGE_2D, 1, 0, float4, uTextures, 32);

PS_OUT main(PS_IN IN)
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
				float alpha = uTextures_image_cis[tid].Sample(uTextures_sampler_cis[tid], IN.v_TextCoord).r;
				sampled = float4(1.0, 1.0, 1.0, alpha);
			}
		}
	}
	OUT.colour = IN.v_Colour * sampled;
	
	return OUT;
}

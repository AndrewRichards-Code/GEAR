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
	MIRU_LOCATION(1, float3, v_TextCoord, TEXCOORD1);
};

//From Application
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 1, 1, float4, uTexture);

PS_OUT main(PS_IN IN)
{
	PS_OUT OUT;
	float dpRange = 0.9999;
	float4 axisColour = uTexture_image_cis.Sample(uTexture_sampler_cis, IN.v_TextCoord);
	
	/*if(dot(normalize(IN.v_TextCoord), float3(-1.0, 0.0, 0.0)) > dpRange) 
		axisColour = float4(1.0, 0.0, 0.0, 1.0);
	else if(dot(normalize(IN.v_TextCoord), float3(0.0, 1.0, 0.0)) > dpRange)
		axisColour = float4(0.0, 1.0, 0.0, 1.0);
	else if(dot(normalize(IN.v_TextCoord), float3(0.0, 0.0, 1.0)) > dpRange)
		axisColour = float4(0.0, 0.0, 1.0, 1.0);
	else if(dot(normalize(IN.v_TextCoord), float3(1.0, 0.0, 0.0)) > dpRange) 
		axisColour = float4(0.0, 1.0, 1.0, 1.0);
	else if(dot(normalize(IN.v_TextCoord), float3(0.0, -1.0, 0.0)) > dpRange) 
		axisColour = float4(1.0, 0.0, 1.0, 1.0);
	else if(dot(normalize(IN.v_TextCoord), float3(0.0, 0.0, -1.0)) > dpRange) 
		axisColour = float4(1.0, 1.0, 0.0, 1.0);*/
	
	OUT.colour = axisColour;
	
	return OUT;
}
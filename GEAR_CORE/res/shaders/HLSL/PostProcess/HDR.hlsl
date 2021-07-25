#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "ColourFunctions.h"

MIRU_UNIFORM_BUFFER(0, 0, HDRInfo, hdrInfo);
MIRU_SUBPASS_INPUT(0, 1, 0, float4, hdrInput);
MIRU_SUBPASS_INPUT(0, 2, 1, float4, emissiveInput);

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
	float4 irradiance = MIRU_SUBPASS_LOAD(hdrInput, IN.v_Position);
	irradiance += MIRU_SUBPASS_LOAD(emissiveInput, IN.v_Position);
	
	//Exposure tone mapping
	float3 mapped = float3(1.0, 1.0, 1.0) - exp(-irradiance.rgb * hdrInfo.exposure);
	
	//Convert Linear to sRGB colour space.
	OUT.colour.rgb = GammaCorrection(hdrInfo.gammaSpace, mapped);
	OUT.colour.a = 1.0;
	
	return OUT;
}
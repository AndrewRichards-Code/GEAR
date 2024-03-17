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

MIRU_UNIFORM_BUFFER(0, 0, ProbeInfo, probeInfo);
MIRU_UNIFORM_BUFFER(1, 0, Model, model);

VS_OUT vs_main_single(VS_IN IN, uint instanceID : SV_InstanceID, out uint renderTargetArrayIndex : SV_RenderTargetArrayIndex )
{
	VS_OUT OUT;
	OUT.position = mul(IN.position, mul(model.modl, mul(probeInfo.view[instanceID], probeInfo.proj[0])));
	OUT.texCoord = IN.texCoord;
	OUT.normal = IN.normal;
	OUT.tangent = IN.tangent;
	OUT.binormal = IN.binormal;
	OUT.colour = IN.colour;
	renderTargetArrayIndex = instanceID;
	return OUT;
}

VS_OUT vs_main_cascades(VS_IN IN, uint viewID : SV_ViewID)
{
	VS_OUT OUT;
	OUT.position = mul(IN.position, mul(model.modl, mul(probeInfo.view[viewID], probeInfo.proj[viewID])));
	OUT.texCoord = IN.texCoord;
	OUT.normal = IN.normal;
	OUT.tangent = IN.tangent;
	OUT.binormal = IN.binormal;
	OUT.colour = IN.colour;
	return OUT;
}

void ps_main()
{
	return;
}
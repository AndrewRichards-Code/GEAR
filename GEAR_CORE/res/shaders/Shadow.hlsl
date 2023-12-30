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
	MIRU_LOCATION(6, uint, v_RenderTargetArrayIndex, SV_RenderTargetArrayIndex);
};
typedef VS_OUT PS_IN;

MIRU_UNIFORM_BUFFER(0, 0, ProbeInfo, probeInfo);
MIRU_UNIFORM_BUFFER(1, 0, Model, model);

VS_OUT vs_main(VS_IN IN, uint instanceID : SV_InstanceID)
{
	VS_OUT OUT;
	OUT.v_Position = mul(IN.positions, mul(model.modl, mul(probeInfo.view[instanceID], probeInfo.proj)));
	OUT.v_TexCoord = IN.texCoords;
	OUT.v_Normal = IN.normals;
	OUT.v_Tangent = IN.tangents;
	OUT.v_Binormal = IN.binormals;
	OUT.v_Colour = IN.colours;
	OUT.v_RenderTargetArrayIndex = instanceID;
	return OUT;
}

void ps_main()
{
	return;
}
#include "msc_common.h"
#include "UniformBufferStructures.h"

//From Application
struct VS_IN
{
	MIRU_LOCATION(0, float4, positions, POSITION0);
	MIRU_LOCATION(1, float2, texCoords, TEXCOORD1);
	MIRU_LOCATION(2, float4, normals, NORMAL2);
	MIRU_LOCATION(3, float4, tangents, TANGENT3);
	MIRU_LOCATION(4, float4, binormals, BINORMAL4);
	MIRU_LOCATION(5, float4, colours, COLOR5);
};

MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);
MIRU_UNIFORM_BUFFER(1, 0, Model, model);

//To Fragment Shader
struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float3, v_TextCoord, TEXCOORD1);
};

VS_OUT main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.v_Position = mul(mul(mul(transpose(camera.proj), transpose(camera.view)), transpose(model.modl)), IN.positions);
	OUT.v_TextCoord = IN.positions.xyz;
	
	return OUT;
}
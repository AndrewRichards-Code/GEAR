#include "msc_common.h"

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

struct Camera
{
	float4x4 proj;
	float4x4 view;
	float4	cameraPosition;
};
MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);

struct Model
{
	float4x4 modl;
	float2 texCoordScale0;
	float2 texCoordScale1;
};
MIRU_UNIFORM_BUFFER(1, 0, Model, model);

//To Fragment Shader
struct VS_OUT
{
	MIRU_LOCATION(0, float4, position, SV_POSITION);
	MIRU_LOCATION(1, float2, texCoord, TEXCOORD1);
	MIRU_LOCATION(2, float3x3, tbn, MATRIX2);
	MIRU_LOCATION(3, float4, worldSpace, POSITION3);
	MIRU_LOCATION(4, float4, vertexToCamera, POSITION4);
	MIRU_LOCATION(5, float4, colour, COLOR5);
};

VS_OUT main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.position = mul(mul(mul(transpose(camera.proj), transpose(camera.view)), transpose(model.modl)), IN.positions);
	OUT.texCoord = float2(model.texCoordScale0.x * IN.texCoords.x, model.texCoordScale0.y * IN.texCoords.y);
	OUT.texCoord += float2(1, 1);
	OUT.texCoord /= 2.0;
	OUT.texCoord.y = 1.0 - OUT.texCoord.y;
	OUT.tbn = float3x3(IN.tangents.xyz, IN.binormals.xyz, IN.normals.xyz);
	OUT.worldSpace = mul(transpose(model.modl), IN.positions);	
	OUT.vertexToCamera = normalize(camera.cameraPosition - OUT.worldSpace);
	OUT.colour = IN.colours;
	
	return OUT;
}

#include "msc_common.h"

//From Application
struct VS_IN
{
	MIRU_LOCATION(0, float4, positions, POSITION0);
};

struct Camera
{
	float4x4 u_Proj;
	float4x4 u_View;
	float4	u_CameraPosition;
};
MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);

struct Model
{
	float4x4 u_Modl;
};
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
	
	OUT.v_Position = mul(mul(mul(transpose(camera.u_Proj), transpose(camera.u_View)), transpose(model.u_Modl)), IN.positions);
	OUT.v_TextCoord = IN.positions.xyz;
	OUT.v_TextCoord.x *= -1;
	
	return OUT;
}
#include "msc_common.h"

//From Application
struct VS_IN
{
	MIRU_LOCATION(0, float4, positions, POSITION0);
	MIRU_LOCATION(1, float2, textCoords, TEXCOORD1);
	MIRU_LOCATION(2, float, textIds, PSIZE2);
	MIRU_LOCATION(3, float4, normals, NORMAL3);
	MIRU_LOCATION(4, float4, colours, COLOR4);
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
	float2 u_UV0;
	float2 u_UV1;
};
MIRU_UNIFORM_BUFFER(1, 0, Model, model);

//To Fragment Shader
struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float2, v_TextCoord, TEXCOORD1);
	MIRU_LOCATION(2, float, v_TextIds, PSIZE2);
	MIRU_LOCATION(3, float4, v_Normal, NORMAL3);
	MIRU_LOCATION(4, float4, v_WorldSpace, POSITION5);
	MIRU_LOCATION(5, float4, v_VertexToCamera, POSITION6);
	MIRU_LOCATION(6, float4, v_Colour, COLOR7);
};

VS_OUT main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.v_Position = mul(mul(mul(transpose(camera.u_Proj), transpose(camera.u_View)), transpose(model.u_Modl)), IN.positions);
	OUT.v_TextCoord.x = model.u_UV0.x * (IN.textCoords.x);
	OUT.v_TextCoord.y = model.u_UV0.y * (1.0 - IN.textCoords.y);
	OUT.v_TextIds = IN.textIds;
	OUT.v_Normal = mul(transpose(model.u_Modl), IN.normals);
	OUT.v_WorldSpace = mul(IN.positions, model.u_Modl);
	OUT.v_VertexToCamera = camera.u_CameraPosition - OUT.v_WorldSpace;
	OUT.v_Colour = IN.colours;
	
	return OUT;
}

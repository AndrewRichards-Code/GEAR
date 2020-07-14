#include "msc_common.h"

//From Application
struct VS_IN
{
	MIRU_LOCATION(0, float4, positions, POSITION0);
	MIRU_LOCATION(1, float2, textCoords, TEXCOORD1);
	MIRU_LOCATION(2, float, textIds, PSIZE);
	MIRU_LOCATION(4, float4, colours, COLOR4);
};

struct Camera
{
	float4x4 u_Proj;
	float4x4 u_View;
	float4	u_CameraPosition;
};
MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);


//To Fragment Shader
struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float2, v_TextCoord, TEXCOORD1);
	MIRU_LOCATION(2, float, v_TextIds, PSIZE2);
	MIRU_LOCATION(3, float4, v_Colour, COLOR3);
};

VS_OUT main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.v_Position = mul(IN.positions, camera.u_Proj);
	OUT.v_TextCoord.x = IN.textCoords.x;
	OUT.v_TextCoord.y = 1.0 - IN.textCoords.y;
	OUT.v_TextIds = IN.textIds;
	OUT.v_Colour = IN.colours;
	
	return OUT;
}
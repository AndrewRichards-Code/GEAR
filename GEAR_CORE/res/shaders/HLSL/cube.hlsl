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
	MIRU_LOCATION(1, float3, v_TextCoord, TEXCOORD1);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);
MIRU_UNIFORM_BUFFER(1, 0, Model, model);
MIRU_UNIFORM_BUFFER(2, 0, SkyboxInfo, skyboxInfo);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 2, 1, float4, skybox);

VS_OUT vs_main(VS_IN IN)
{
	VS_OUT OUT;
	
	OUT.v_Position = mul(mul(mul(transpose(camera.proj), transpose(camera.view)), transpose(model.modl)), IN.positions);
	OUT.v_TextCoord = IN.positions.xyz;
	
	return OUT;
}

PS_OUT ps_main(PS_IN IN)
{
	PS_OUT OUT;
	
	float4 irradiance = skybox_ImageCIS.Sample(skybox_SamplerCIS, IN.v_TextCoord);
	float4 mapped = float4(1.0, 1.0, 1.0, 1.0) - exp(-irradiance * skyboxInfo.exposure);
	OUT.colour = pow(mapped, float4(1.0/skyboxInfo.gamma, 1.0/skyboxInfo.gamma, 1.0/skyboxInfo.gamma, 1.0/skyboxInfo.gamma));
	OUT.colour.a = 1.0;
	
	return OUT;
}
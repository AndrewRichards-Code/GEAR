#include "msc_common.h"
#include "UniformBufferStructures.h"

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
MIRU_UNIFORM_BUFFER(0, 1, SkyboxInfo, skyboxInfo);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 0, 2, float4, skybox);

//Helper Functions
static const float PI = 3.1415926535897932384626433832795;

float2 CubemapToEquirectangularTextCoords(float3 v)
{
	float3 norm_v = normalize(v);
	float theta = asin(norm_v.y);
	float phi = atan2(norm_v.z, norm_v.x);
	float2 uv = float2(phi/(2 * PI), -((theta/PI) + 0.5));
	return uv;
}

PS_OUT main(PS_IN IN)
{
	PS_OUT OUT;
	
	float4 irradiance = skybox_ImageCIS.Sample(skybox_SamplerCIS, CubemapToEquirectangularTextCoords(IN.v_TextCoord));
	float4 mapped = float4(1.0, 1.0, 1.0, 1.0) - exp(-irradiance * skyboxInfo.exposure);
	OUT.colour = pow(mapped, float4(1.0/skyboxInfo.gamma, 1.0/skyboxInfo.gamma, 1.0/skyboxInfo.gamma, 1.0/skyboxInfo.gamma));
	OUT.colour.a = 1.0; 
	
	return OUT;
}
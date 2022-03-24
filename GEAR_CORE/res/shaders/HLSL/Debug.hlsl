#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "CubeFunctions.h"

struct VS_IN
{
	uint vertex_id : SV_VertexID;
};

struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float4, v_Colour, COLOR1);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);

VS_OUT vs_coordinate_axes(VS_IN IN)
{
	VS_OUT OUT;
	
	if (IN.vertex_id >= 0 && IN.vertex_id < 6)
	{
		switch (IN.vertex_id)
		{
		case 0:
		{
			OUT.v_Position = float4(0.0, 0.0, 0.0, 1.0);
			OUT.v_Colour = float4(1.0, 0.0, 0.0, 1.0);
			break;
		}
		case 1:
		{
			OUT.v_Position = float4(1.0, 0.0, 0.0, 1.0);
			OUT.v_Colour = float4(1.0, 0.0, 0.0, 1.0);
			break;
		}
		case 2:
		{
			OUT.v_Position = float4(0.0, 0.0, 0.0, 1.0);
			OUT.v_Colour = float4(0.0, 1.0, 0.0, 1.0);
			break;
		}
		case 3:
		{
			OUT.v_Position = float4(0.0, 1.0, 0.0, 1.0);
			OUT.v_Colour = float4(0.0, 1.0, 0.0, 1.0);
			break;
		}
		case 4:
		{
			OUT.v_Position = float4(0.0, 0.0, 0.0, 1.0);
			OUT.v_Colour = float4(0.0, 0.0, 1.0, 1.0);
			break;
		}
		case 5:
		{
			OUT.v_Position = float4(0.0, 0.0, 1.0, 1.0);
			OUT.v_Colour = float4(0.0, 0.0, 1.0, 1.0);
			break;
		}
		}
	}
	
	float4x4 transform = float4x4(
		float4(0.1, 0, 0, 0),
		float4(0, 0.1, 0, 0),
		float4(0, 0, 0.1, 0),
		float4(0, 0, 0, 1.0)
	);
	
	float4x4 invCameraTranslation = float4x4(
		float4(1, 0, 0, camera.position.x),
		float4(0, 1, 0, camera.position.y),
		float4(0, 0, 1, camera.position.z),
		float4(0, 0, 0, 1)
	);
	
	float4x4 cameraOrientation = mul(transpose(camera.view), transpose(invCameraTranslation));
	
	
	OUT.v_Position = mul(mul(mul(transpose(camera.proj), cameraOrientation), transpose(transform)), OUT.v_Position);
	
	return OUT;
}

PS_OUT ps_coordinate_axes(PS_IN IN)
{
	PS_OUT OUT;
	
	OUT.colour = IN.v_Colour;
	
	return OUT;
}

MIRU_IMAGE_2D(0, 0, float4, sourceImage);

VS_OUT vs_copy(VS_IN IN)
{
	VS_OUT OUT;
	OUT.v_Position = float4(float2((IN.vertex_id << 1) & 2, IN.vertex_id & 2) * 2.0f - 1.0f, 0.0f, 1.0f);
	return OUT;
}

PS_OUT ps_copy(PS_IN IN)
{
	PS_OUT OUT;
	OUT.colour = sourceImage.Load(int3(IN.v_Position.xy, 0));
	
	return OUT;
}

MIRU_UNIFORM_BUFFER(0, 0, Camera, debugCamera);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 1, float4, cubemap);

VS_OUT vs_show_cubemap(VS_IN IN)
{
	VS_OUT OUT;
	switch (IN.vertex_id)
	{
		case 0:
		case 5:
			OUT.v_Position = float4(-1.0, -1.0, 1.0, 1.0);
			break;
		case 1:
			OUT.v_Position = float4(+1.0, -1.0, 1.0, 1.0);
			break;
		case 2:
		case 3:
			OUT.v_Position = float4(+1.0, +1.0, 1.0, 1.0);
			break;
		case 4:
			OUT.v_Position = float4(-1.0, +1.0, 1.0, 1.0);
			break;
	}
	OUT.v_Colour = OUT.v_Position;

	return OUT;
};

PS_OUT ps_show_cubemap(PS_IN IN)
{
	PS_OUT OUT;
	float4x4 viewProj = mul(debugCamera.view, debugCamera.proj);
	float4x4 viewProj_Inv = inverse(viewProj);
	float3 view = normalize(mul(IN.v_Colour, viewProj_Inv)).xyz;
	float depth = 1.0 - cubemap_ImageCIS.Sample(cubemap_SamplerCIS, view).x;
	float4 depthOutput = 100.0 * float4(depth, depth, depth, 1.0);
	float4 faceColour = CubemapFaceColour(UVWToFaceIndex(view));
	OUT.colour = lerp(depthOutput, faceColour, 0.1);
	return OUT;
}
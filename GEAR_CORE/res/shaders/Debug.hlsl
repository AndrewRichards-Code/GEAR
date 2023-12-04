#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "CubeFunctions.h"
#include "Depth.h"

struct VS_IN
{
	uint vertex_id : SV_VertexID;
};

struct VS_OUT
{
	MIRU_LOCATION(0, float4, v_Position, SV_POSITION);
	MIRU_LOCATION(1, float4, v_NDCPosition, POSITION1);
	MIRU_LOCATION(2, float4, v_Colour, COLOR2);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

//Co-ordinate Axes
MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);

//Copy
MIRU_IMAGE_2D(0, 0, float4, sourceImage);

//ShowDepth
MIRU_UNIFORM_BUFFER(0, 0, Camera, debugCamera);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D, 0, 1, float4, image2D);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE, 0, 1, float4, cubemap);

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
	
	float aspectRatio = abs(1.0 / (camera.proj[0][0] / camera.proj[1][1]));
	float scale = 0.1;
	float scaleNDCOffset = scale * 1.125;
	bool reverseDepth = camera.proj[2][2] < 0.0;

	float4x4 viewProj = mul(camera.view, camera.proj);
	float4x4 view_Inv = inverse(camera.view);
	float3 view = normalize(mul(float4(0.0, 0.0, 1.0, 0.0), view_Inv).xyz);

	float4x4 transform = transpose(float4x4(
		float4(scale, 0.0, 0.0, view.x + camera.position.x),
		float4(0.0, scale, 0.0, view.y + camera.position.y),
		float4(0.0, 0.0, scale, view.z + camera.position.z),
		float4(0.0, 0.0, 0.0, 1.0)
	));
	float4x4 ndcTransform = transpose(float4x4(
		float4(1.0, 0.0, 0.0, -(1.0 - saturate(scaleNDCOffset / aspectRatio))),
		float4(0.0, 1.0, 0.0, -(1.0 - scaleNDCOffset)),
		float4(0.0, 0.0, 1.0, 0.0),
		float4(0.0, 0.0, 0.0, 1.0)
	));

	//Need for differing NDC spaces in D3D12 and Vulkan after the Camera projection matrix is applied.
	#if MIRU_VULKAN
	ndcTransform[3][1] *= -1;
	#endif

	OUT.v_Position = mul(OUT.v_Position, mul(transform, mul(camera.view, mul(camera.proj, ndcTransform))));
	
	return OUT;
}

PS_OUT ps_coordinate_axes(PS_IN IN)
{
	PS_OUT OUT;
	
	OUT.colour = IN.v_Colour;
	
	return OUT;
}


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


VS_OUT vs_show_depth_image2D(VS_IN IN)
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
	OUT.v_NDCPosition = OUT.v_Position;

	return OUT;
}

PS_OUT ps_show_depth_image2D(PS_IN IN)
{
	PS_OUT OUT;
	float2 textureCoords = (IN.v_NDCPosition.xy / 2.0) + float2(0.5, 0.5);
	
	//Texture flipped between D3D12 and Vulkan. Usually handled by our projection matrix, but we don't have one here.
	//https://github.com/gpuweb/gpuweb/issues/416
#if MIRU_D3D12
	textureCoords.y = 1.0 - textureCoords.y; 
#endif
	
	float depth = LineariseDepth(image2D_ImageCIS.Sample(image2D_SamplerCIS, textureCoords).x, debugCamera.proj);
	OUT.colour = float4(depth, depth, depth, 1.0);
	return OUT;
}

VS_OUT vs_show_depth_cubemap(VS_IN IN)
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
	OUT.v_NDCPosition = OUT.v_Position;

	return OUT;
}

PS_OUT ps_show_depth_cubemap(PS_IN IN)
{
	PS_OUT OUT;
	float4x4 viewProj = mul(debugCamera.view, debugCamera.proj);
	float4x4 viewProj_Inv = inverse(viewProj);
	float3 view = normalize(mul(IN.v_NDCPosition, viewProj_Inv)).xyz;
	
	float depth = /*LineariseDepth(*/cubemap_ImageCIS.Sample(cubemap_SamplerCIS, view).x/*, debugCamera.proj)*/;
	float minValue = 0.995;
	//depth = ((1.0 - minValue) * depth) + minValue;
	depth = (depth - minValue) / (1.0 - minValue);
	float4 depthOutput = float4(depth, depth, depth, 1.0);
	float4 faceColour = CubemapFaceColour(UVWToFaceIndex(view));
	
	OUT.colour = lerp(depthOutput, faceColour, 0.1);
	return OUT;
}
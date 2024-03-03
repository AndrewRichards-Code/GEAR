#include "msc_common.h"
#include "UniformBufferStructures.h"
#include "CubeFunctions.h"
#include "Depth.h"

//General
struct VS_IN
{
	uint vertexID : SV_VertexID;
};

struct VS_OUT
{
	MIRU_LOCATION(0, float4, position, SV_POSITION);
	MIRU_LOCATION(1, float4, colour, COLOR1);
};
typedef VS_OUT PS_IN;

struct PS_OUT
{
	MIRU_LOCATION(0, float4, colour, SV_TARGET0);
};

//Co-ordinate Axes
MIRU_UNIFORM_BUFFER(0, 0, Camera, camera);

VS_OUT vs_coordinate_axes(VS_IN IN)
{
	VS_OUT OUT;
	
	if (IN.vertexID >= 0 && IN.vertexID < 6)
	{
		switch (IN.vertexID)
		{
		case 0:
		{
			OUT.position = float4(0.0, 0.0, 0.0, 1.0);
			OUT.colour = float4(1.0, 0.0, 0.0, 1.0);
			break;
		}
		case 1:
		{
			OUT.position = float4(1.0, 0.0, 0.0, 1.0);
			OUT.colour = float4(1.0, 0.0, 0.0, 1.0);
			break;
		}
		case 2:
		{
			OUT.position = float4(0.0, 0.0, 0.0, 1.0);
			OUT.colour = float4(0.0, 1.0, 0.0, 1.0);
			break;
		}
		case 3:
		{
			OUT.position = float4(0.0, 1.0, 0.0, 1.0);
			OUT.colour = float4(0.0, 1.0, 0.0, 1.0);
			break;
		}
		case 4:
		{
			OUT.position = float4(0.0, 0.0, 0.0, 1.0);
			OUT.colour = float4(0.0, 0.0, 1.0, 1.0);
			break;
		}
		case 5:
		{
			OUT.position = float4(0.0, 0.0, 1.0, 1.0);
			OUT.colour = float4(0.0, 0.0, 1.0, 1.0);
			break;
		}
		}
	}
	
	float aspectRatio = abs(1.0 / (camera.proj[0][0] / camera.proj[1][1]));
	float scale = 0.1;
	float clipSpaceOffset = scale * 1.125;
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
	float4x4 clipSpaceTransform = transpose(float4x4(
		float4(1.0, 0.0, 0.0, -(1.0 - saturate(clipSpaceOffset / aspectRatio))),
		float4(0.0, 1.0, 0.0, -(1.0 - clipSpaceOffset)),
		float4(0.0, 0.0, 1.0, 0.0),
		float4(0.0, 0.0, 0.0, 1.0)
	));

	//Need for differing NDC spaces in D3D12 and Vulkan after the Camera projection matrix is applied.
	#if MIRU_VULKAN
	clipSpaceTransform[3][1] *= -1;
	#endif

	OUT.position = mul(OUT.position, mul(transform, mul(camera.view, mul(camera.proj, clipSpaceTransform))));
	
	return OUT;
}

PS_OUT ps_coordinate_axes(PS_IN IN)
{
	PS_OUT OUT;
	
	OUT.colour = IN.colour;
	
	return OUT;
}

//Copy
MIRU_IMAGE_2D(0, 0, float4, sourceImage);

VS_OUT vs_copy(VS_IN IN)
{
	VS_OUT OUT;
	OUT.position = float4(float2((IN.vertexID << 1) & 2, IN.vertexID & 2) * 2.0f - 1.0f, 0.0f, 1.0f);
	return OUT;
}

PS_OUT ps_copy(PS_IN IN)
{
	PS_OUT OUT;
	OUT.colour = sourceImage.Load(int3(IN.position.xy, 0));
	
	return OUT;
}


//ShowDepth
struct VS_OUT_2
{
	MIRU_LOCATION(0, float4, position, SV_POSITION);
	MIRU_LOCATION(1, float4, clipPosition, POSITION1);
};
typedef VS_OUT_2 PS_IN_2;

MIRU_UNIFORM_BUFFER(0, 0, Camera, debugCamera);
MIRU_UNIFORM_BUFFER(0, 1, DebugProbeInfo, debugProbeInfo);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_2D_ARRAY, 0, 2, float4, image2DArray);
MIRU_COMBINED_IMAGE_SAMPLER(MIRU_IMAGE_CUBE_ARRAY, 0, 2, float4, cubemapArray);

VS_OUT_2 vs_show_depth_image2D(VS_IN IN)
{
	VS_OUT_2 OUT;
	switch (IN.vertexID)
	{
		case 0:
		case 5:
			OUT.position = float4(-1.0, -1.0, 1.0, 1.0);
			break;
		case 1:
			OUT.position = float4(+1.0, -1.0, 1.0, 1.0);
			break;
		case 2:
		case 3:
			OUT.position = float4(+1.0, +1.0, 1.0, 1.0);
			break;
		case 4:
			OUT.position = float4(-1.0, +1.0, 1.0, 1.0);
			break;
	}
	OUT.clipPosition = OUT.position;
	
	//NDC flipped between D3D12 and Vulkan. Usually handled by our projection matrix, but we don't have one here.
	//https://github.com/gpuweb/gpuweb/issues/416
#if MIRU_D3D12
	OUT.clipPosition.y *= -1.0;
#endif

	return OUT;
}

PS_OUT ps_show_depth_image2D(PS_IN_2 IN, uint viewID : SV_ViewID)
{
	PS_OUT OUT;
	float3 textureCoords = float3((IN.clipPosition.xy / 2.0) + float2(0.5, 0.5), float(viewID));
	
	float depth = image2DArray_ImageCIS.Sample(image2DArray_SamplerCIS, textureCoords).x;
	depth = clamp(depth, debugProbeInfo.minDepth, debugProbeInfo.maxDepth) / (debugProbeInfo.maxDepth - debugProbeInfo.minDepth);
	float4 depthOutput = float4(depth, depth, depth, 1.0);
	if (debugProbeInfo.showColourCubemap)
	{
		float4x4 viewProj = mul(debugProbeInfo.view, debugProbeInfo.proj);
		float4x4 viewProj_Inv = inverse(viewProj);
		float3 view = normalize(mul(IN.clipPosition, viewProj_Inv)).xyz;
		float4 faceColour = CubemapFaceColour(UVWToFaceIndex(view));
		OUT.colour = lerp(depthOutput, faceColour, 0.1);
	}
	else
	{
		OUT.colour = depthOutput;
	}
	return OUT;
}

VS_OUT_2 vs_show_depth_cubemap(VS_IN IN)
{
	VS_OUT_2 OUT;
	switch (IN.vertexID)
	{
		case 0:
		case 5:
			OUT.position = float4(-1.0, -1.0, 1.0, 1.0);
			break;
		case 1:
			OUT.position = float4(+1.0, -1.0, 1.0, 1.0);
			break;
		case 2:
		case 3:
			OUT.position = float4(+1.0, +1.0, 1.0, 1.0);
			break;
		case 4:
			OUT.position = float4(-1.0, +1.0, 1.0, 1.0);
			break;
	}
	OUT.clipPosition = OUT.position;

	return OUT;
}

PS_OUT ps_show_depth_cubemap(PS_IN_2 IN, uint viewID : SV_ViewID)
{
	PS_OUT OUT;
	float4x4 viewProj = mul(debugCamera.view, debugCamera.proj);
	float4x4 viewProj_Inv = inverse(viewProj);
	float3 view = normalize(mul(IN.clipPosition, viewProj_Inv)).xyz;
	
	float depth = cubemapArray_ImageCIS.Sample(cubemapArray_SamplerCIS, float4(view, viewID)).x;
	depth = clamp(depth, debugProbeInfo.minDepth, debugProbeInfo.maxDepth) / (debugProbeInfo.maxDepth - debugProbeInfo.minDepth);
	float4 depthOutput = float4(depth, depth, depth, 1.0);
	if (debugProbeInfo.showColourCubemap)
	{
		float4 faceColour = CubemapFaceColour(UVWToFaceIndex(view));
		OUT.colour = lerp(depthOutput, faceColour, 0.1);
	}
	else
	{
		OUT.colour = depthOutput;
	}
	return OUT;
}

//DrawBoxes
MIRU_STRUCTURED_BUFFER(0, 1, Model, modelMatrices);

VS_OUT vs_main_boxes(VS_IN IN, uint instancedID : SV_InstanceID)
{
	VS_OUT OUT;
	
	const float size = 0.125;
	
	switch (IN.vertexID)
	{
		case 0:
		case 7:
		case 16:
			OUT.position = float4(-size, -size, -size, 1.0); //0
			break;
		case 1:
		case 2:
		case 18:
			OUT.position = float4(+size, -size, -size, 1.0); //1
			break;
		case 3:
		case 4:
		case 20:
			OUT.position = float4(+size, +size, -size, 1.0); //2
			break;
		case 5:
		case 6:
		case 22:
			OUT.position = float4(-size, +size, -size, 1.0); //3
			break;
		case 8:
		case 15:
		case 17:
			OUT.position = float4(-size, -size, +size, 1.0); //4
			break;
		case 9:
		case 10:
		case 19:
			OUT.position = float4(+size, -size, +size, 1.0); //5
			break;
		case 11:
		case 12:
		case 21:
			OUT.position = float4(+size, +size, +size, 1.0); //6
			break;
		case 13:
		case 14:
		case 23:
			OUT.position = float4(-size, +size, +size, 1.0); //7
			break;
	}
	
	Model model = modelMatrices[instancedID];
	OUT.position = mul(OUT.position, mul(model.modl, mul(camera.view, camera.proj)));
	OUT.colour = float4(model.texCoordScale0, model.texCoordScale1);
	
	return OUT;
}

PS_OUT ps_main_boxes(PS_IN IN)
{
	PS_OUT OUT;
	OUT.colour = IN.colour;
	
	return OUT;
}
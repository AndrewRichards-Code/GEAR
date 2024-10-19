#pragma once

// https://developer.nvidia.com/content/depth-precision-visualized
float LineariseDepth(float storedDepthValue, float4x4 projectionMatrix)
{
	// Orthographic projections are linear by default.
	if (projectionMatrix[3][3] == 1.0)
		return storedDepthValue;

	float a = projectionMatrix[3][2];
	float b = projectionMatrix[2][2];
	float d = storedDepthValue;

	return a / (d - b);
}

float4 PerspectiveDivide(float4 worldSpacePosition, float4x4 view, float4x4 proj)
{
	float4 projectedPosition = mul(worldSpacePosition, mul(view, proj));
	float4 ndcPosition = projectedPosition / projectedPosition.w; //Do the perspective divide.
	//NDC flipped between D3D12 and Vulkan. https://github.com/gpuweb/gpuweb/issues/416
#if MIRU_D3D12
	ndcPosition.y *= -1.0;
#endif
	return ndcPosition;
}
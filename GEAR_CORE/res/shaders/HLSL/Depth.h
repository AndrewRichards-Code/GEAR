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

	return a / (storedDepthValue - b);
}

float2 GetNearFarValues(float4x4 projectionMatrix)
{
	float a = projectionMatrix[3][2];
	float b = projectionMatrix[2][2];

	float near = abs(a / b);
	float far = (near * b) / (b - 1.0f);

	return float2(near, far);
}
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
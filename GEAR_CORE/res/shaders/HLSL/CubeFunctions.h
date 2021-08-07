#ifndef CUBE_FUNCTIONS_H
#define CUBE_FUNCTIONS_H

float3 GetLookupUVW(uint3 threadID, float2 imageDimension)
{
	float2 faceTC = threadID.xy / imageDimension;
	faceTC.y = 1.0 - faceTC.y;
	float2 uv = 2.0 * faceTC - float2(1.0, 1.0);

	//Select vector based on cubemap face index.
	float3 result = float3(0.0, 0.0, 0.0);

	switch (threadID.z)
	{
	case 0: result = float3(1.0, uv.y, -uv.x); break;
	case 1: result = float3(-1.0, uv.y, uv.x); break;
	case 2: result = float3(uv.x, 1.0, -uv.y); break;
	case 3: result = float3(uv.x, -1.0, uv.y); break;
	case 4: result = float3(uv.x, uv.y, 1.0); break;
	case 5: result = float3(-uv.x, uv.y, -1.0); break;
	}

	return normalize(result);
}

float2 CubemapToEquirectangularTextCoords(float3 v)
{
	const float PI = 3.1415926535897932384626433832795;
	const float TAU = 2.0 * PI;

	float3 norm_v = normalize(v);
	float theta = asin(norm_v.y);
	float phi = atan2(norm_v.z, norm_v.x);
	float2 uv = float2(phi / TAU, -((theta / PI) + 0.5));
	return uv;
}

float4 CubemapFaceColour(uint faceIndex)
{
	float4 result = float4(0.0, 0.0, 0.0, 1.0);

	switch (faceIndex)
	{
	case 0: result = float4(1.0, 0.0, 0.0, 1.0); break;
	case 1: result = float4(0.0, 1.0, 1.0, 1.0); break;
	case 2: result = float4(0.0, 1.0, 0.0, 1.0); break;
	case 3: result = float4(1.0, 0.0, 1.0, 1.0); break;
	case 4: result = float4(0.0, 0.0, 1.0, 1.0); break;
	case 5: result = float4(1.0, 1.0, 0.0, 1.0); break;
	}

	return result;
}

//Compute orthonormal basis for converting from tanget/shading space to world space.
void ComputeBasisVectors(const float3 N, out float3 B, out float3 T)
{
	//Branchless select non-degenerate T.
	T = cross(N, float3(0.0, 1.0, 0.0));
	T = lerp(cross(N, float3(1.0, 0.0, 0.0)), T, step(0.00001, dot(T, T)));

	T = normalize(T);
	B = normalize(cross(N, T));
}

//Convert point from tangent/shading space to world space.
float3 TangentToWorld(const float3 v, const float3 N, const float3 B, const float3 T)
{
	return B * v.x + T * v.y + N * v.z;
}

#endif // CUBE_FUNCTIONS_H

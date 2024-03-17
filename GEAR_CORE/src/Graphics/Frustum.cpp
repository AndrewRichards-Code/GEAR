#include "Frustum.h"
#include "Picker.h"
#include <algorithm>

using namespace gear;
using namespace graphics;
using namespace mars;

Frustum::Frustum()
{
	corners[0] = float3(-1.0f, -1.0f, +0.0f);
	corners[1] = float3(+1.0f, -1.0f, +0.0f);
	corners[2] = float3(+1.0f, +1.0f, +0.0f);
	corners[3] = float3(-1.0f, +1.0f, +0.0f);
	corners[4] = float3(-1.0f, -1.0f, +1.0f);
	corners[5] = float3(+1.0f, -1.0f, +1.0f);
	corners[6] = float3(+1.0f, +1.0f, +1.0f);
	corners[7] = float3(-1.0f, +1.0f, +1.0f);
}

Frustum::Frustum(const float4x4& proj, const float4x4& view)
{
	*this = Frustum();
	float4x4 invProjView = float4x4::Inverse(proj * view);
	bool reverseDepth = std::signbit(proj.k);
	for (float3& corner : corners)
	{
		if (reverseDepth)
			corner.z = (corner.z == 0.0f ? 1.0f : 0.0f);

		float4 clipSpaceCorner = invProjView * float4(corner, 1.0f);
		corner = clipSpaceCorner * (1.0f / clipSpaceCorner.w);
	}
}

Frustum::~Frustum()
{
}

void Frustum::ScaleDistancesForNearAndFar(float near, float far)
{
	for (uint32_t i = 0; i < 4; i++) 
	{
		float3 distance = corners[i + 4] - corners[i];
		corners[i + 4] = corners[i + 0] + (distance * far);
		corners[i + 0] = corners[i + 0] + (distance * near);
	}
}

const float3 Frustum::GetCentre() const
{
	float3 centre(0.0f, 0.0f, 0.0f);
	for (const float3& corner : corners)
	{
		centre += corner;
	}
	centre /= static_cast<float>(corners.size());

	return centre;
}

const float Frustum::GetMaxRadius() const
{
	float radius = 0.0f;
	const float3 centre = GetCentre();

	for (const float3& corner : corners)
	{
		float3 distance = corner - centre;
		radius = std::max(distance.Length<float>(), radius);
	}

	return radius;
}

const BoundingBox Frustum::GetExtents() const
{
	const float radius = GetMaxRadius();
	return { -float3(radius, radius, radius), float3(radius, radius, radius) };
}

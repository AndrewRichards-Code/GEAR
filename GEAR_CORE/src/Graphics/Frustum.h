#pragma once
#include "gear_core_common.h"

namespace gear 
{
	namespace graphics
	{
		struct BoundingBox
		{
			mars::float3 min;
			mars::float3 max;
		};

		class Frustum
		{
		public:
			Frustum();
			Frustum(const mars::float4x4& proj, const mars::float4x4& view);
			~Frustum();

			//Parameters are normalised value from the near to the far plane.
			void ScaleDistancesForNearAndFar(float near, float far);

			const mars::float3 GetCentre() const;
			const float GetMaxRadius() const;
			const BoundingBox GetExtents() const;

		public:
			std::array<mars::float3, 8> corners;

		};
	}
}
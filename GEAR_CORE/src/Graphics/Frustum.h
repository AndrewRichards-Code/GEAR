#pragma once
#include "gear_core_common.h"
#include "Graphics/Picker.h"

namespace gear 
{
	namespace graphics
	{
		class GEAR_API Frustum
		{
			//Methods
		public:
			Frustum();
			Frustum(const mars::float4x4& proj, const mars::float4x4& view);
			~Frustum();

			//Parameters are normalised value from the near to the far plane.
			void ScaleDistancesForNearAndFar(float near, float far);

			const mars::float3 GetCentre() const;
			const float GetMaxRadius() const;
			const BoundingBox GetExtents() const;

			//Members
		public:
			std::array<mars::float3, 8> corners;

		};
	}
}
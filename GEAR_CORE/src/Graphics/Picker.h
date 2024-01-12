#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace objects
	{
		class Camera;
		class Model;
	}
	namespace graphics
	{
		class GEAR_API Picker
		{
		public:
			struct Ray
			{
				mars::float3 origin;
				mars::float3 direction;
				float length;
			};


			static Ray GetPickingRay(const Ref<objects::Camera>& camera, const mars::float2& pixelCoords, const mars::float2& viewportSize);
			
			static miru::base::AabbData GetAABB(const Ref<objects::Model>& model);
			static float RayIntersectsAABB(const miru::base::AabbData& aabb, const Ray& ray);
			static float RayIntersectsModelGeometry(const Ref<objects::Model>& model, const Ray& ray);

			static Ref<objects::Model> GetNearestModel(const std::vector<Ref<objects::Model>>& models, const Ref<objects::Camera>& camera, const mars::float2& pixelCoords, const mars::float2& viewportSize);
		};
	}
}
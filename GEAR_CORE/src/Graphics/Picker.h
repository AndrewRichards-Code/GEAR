#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace objects
	{
		class Camera;
		class Model;
		class Light;
	}
	namespace graphics
	{
		struct GEAR_API Ray
		{
			mars::float3 origin;
			mars::float3 direction;
			float length;
		};

		struct GEAR_API BoundingBox
		{
			mars::float3 min;
			mars::float3 max;
		};

		class GEAR_API Picker
		{
			//Methods
		public:
			static Ray GetPickingRay(const Ref<objects::Camera>& camera, const mars::float2& pixelCoords, const mars::float2& viewportSize);
			
			static BoundingBox GetAABB(const Ref<objects::Model>& model);
			static float RayIntersectsAABB(const BoundingBox& aabb, const Ray& ray);
			static float RayIntersectsModelGeometry(const Ref<objects::Model>& model, const Ray& ray);

			static Ref<objects::Model> GetNearestModel(const std::vector<Ref<objects::Model>>& models, const Ref<objects::Camera>& camera, const mars::float2& pixelCoords, const mars::float2& viewportSize);
			static Ref<objects::Light> GetNearestLight(const std::vector<Ref<objects::Light>>& lights, const Ref<objects::Camera>& camera, const mars::float2& pixelCoords, const mars::float2& viewportSize);
		};
	}
}
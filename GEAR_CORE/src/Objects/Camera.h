#pragma once
#include "Objects/ObjectInterface.h"
#include "Objects/Transform.h"
#include "Graphics/Colour.h"
#include "Graphics/Uniformbuffer.h"

#undef far
#undef near

namespace gear 
{
	namespace objects
	{
		class GEAR_API Camera : public ObjectInterface, public ObjectViewInterface
		{
		public:
			enum class ProjectionType : uint32_t
			{
				PERSPECTIVE,
				ORTHOGRAPHIC
			};

			struct OrthographicParameters
			{
				float	left;
				float	right;
				float	bottom;
				float	top;
				float	near;
				float	far;
			};
			struct PerspectiveParameters
			{
				double	horizonalFOV;
				float	aspectRatio;
				float	zNear;
				float	zFar;
			};
			

			struct CreateInfo : public ObjectInterface::CreateInfo
			{
				ProjectionType	projectionType;
				union
				{
					OrthographicParameters	orthographicParams;
					PerspectiveParameters	perspectiveParams;
				};
			};

		private:
			typedef graphics::UniformBufferStructures::Camera CameraUB;
			Ref<graphics::Uniformbuffer<CameraUB>> m_CameraUB;

		public:
			CreateInfo m_CI;

			mars::float3 m_Direction;
			mars::float3 m_Up;
			mars::float3 m_Right;

		public:
			Camera(CreateInfo* pCreateInfo);
			~Camera();

			const Ref<graphics::Uniformbuffer<CameraUB>>& GetCameraUB() const { return m_CameraUB; };

			//Update the camera from the current state of Camera::CreateInfo m_CI.
			void Update(const Transform& transform) override;

			//The view matrix is already inversed and should be passed unmodified to a UniformBuffer's view matrix member.
			static mars::float4x4 GetCubemapFaceViewMatrix(uint32_t faceIndex, const mars::float3& translation = mars::float3(0.0f, 0.0f, 0.0f));

		protected:
			bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

		private:
			void DefineProjection();
			void DefineView(const Transform& transform);
			void SetPosition(const Transform& transform);
			void InitialiseUB();
		};

		struct BoundingBox
		{
			mars::float3 min;
			mars::float3 max;
		};

		class Frustrum
		{
		public:
			Frustrum();
			Frustrum(const mars::float4x4& proj, const mars::float4x4& view);
			~Frustrum();

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